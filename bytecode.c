#include "bytecode.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "lang_types.h"
#include "utils.h"

static inline void chunk_realloc(struct chunk* chunk, size_t newsize);

static inline void bcchunk_write_code(struct bytecode_chunk* chunk, byte_t byte);
static inline void bcchunk_write_data(struct bytecode_chunk* chunk, byte_t byte);

static inline size_t instruction_debug(const struct bytecode_chunk* chunk, size_t offset);
static inline void print_instruction_debug(const char* name, size_t offset);
static inline size_t simple_instruction_debug(const char* name, const struct bytecode_chunk* chunk, size_t offset);
static inline size_t constant_instruction_debug(const char* name, const struct bytecode_chunk* chunk, size_t offset);

static void parse_ast_bin_expr(ast_node* node, struct bytecode_chunk* chunk);

static inline value_t readvalue(const struct bytecode_chunk* chunk, size_t code_offset);

void chunk_init(struct chunk* chunk){
    chunk->size = 0;
    chunk->capacity = CHUNK_BASE_CAPACITY;
    if((chunk->data = malloc(CHUNK_BASE_CAPACITY)) == NULL)
        fatal_printf("malloc() returned NULL!\n");
}

void chunk_free(struct chunk* chunk){
    chunk->size = 0;
    chunk->capacity = 0;
    free(chunk->data);
    chunk->data = NULL;
}

void chunk_write(struct chunk* chunk, byte_t byte){
    if (chunk->capacity <= chunk->size)
        chunk_realloc(chunk, chunk->capacity + CHUNK_BASE_CAPACITY);
    chunk->data[chunk->size++] = byte;
}

void chunk_write_number(struct chunk* chunk, int number){
    if (chunk->capacity < chunk->size + sizeof(number))
        chunk_realloc(chunk, chunk->capacity + CHUNK_BASE_CAPACITY);
    *((int*)(chunk->data + chunk->size)) = number;
    chunk->size += sizeof(number);
}

void chunk_write_value(struct chunk* chunk, value_t val){
    if (chunk->capacity < chunk->size + sizeof(val.as))
        chunk_realloc(chunk, chunk->capacity + CHUNK_BASE_CAPACITY);
    *((union _inner_value_t*)(chunk->data + chunk->size)) = val.as;
    chunk->size += sizeof(val.as);
}

static inline void chunk_realloc(struct chunk* chunk, size_t newsize){
    if((chunk->capacity = newsize) == 0)
        fatal_printf("chunk_realloc(): newsize = 0!\n");
    if((chunk->data = realloc(chunk->data, newsize)) == NULL)
        fatal_printf("realloc() returned NULL!\n");
    chunk->size = chunk->size > newsize ? chunk->size : newsize;
}

void bcchunk_init(struct bytecode_chunk* chunk){
    chunk_init(&chunk->_code);
    chunk_init(&chunk->_data);
}

void bcchunk_free(struct bytecode_chunk* chunk){
    chunk_free(&chunk->_code);
    chunk_free(&chunk->_data);
}

static inline void bcchunk_write_code(struct bytecode_chunk* chunk, byte_t byte){
    chunk_write(&chunk->_code, byte);
}


static inline void bcchunk_write_data(struct bytecode_chunk* chunk, byte_t byte){
    chunk_write(&chunk->_data, byte);
}

static inline size_t instruction_debug(const struct bytecode_chunk* chunk, size_t offset){
    switch (chunk->_code.data[offset]) {
        case OP_RETURN: return simple_instruction_debug("OP_RETURN",chunk, offset);
        case OP_ADD: return simple_instruction_debug("OP_ADD",chunk, offset);
        case OP_SUB: return simple_instruction_debug("OP_SUB",chunk, offset);
        case OP_DIV: return simple_instruction_debug("OP_DIV",chunk, offset);
        case OP_MUL: return simple_instruction_debug("OP_MUL",chunk, offset);
        case OP_AND: return simple_instruction_debug("OP_AND",chunk, offset);
        case OP_OR: return simple_instruction_debug("OP_OR",chunk, offset);
        case OP_XOR: return simple_instruction_debug("OP_XOR",chunk, offset);
        case OP_NOT: return simple_instruction_debug("OP_NOT",chunk, offset);
        case OP_NUMBER: return constant_instruction_debug("OP_NUMBER",chunk, offset);
        case OP_BOOLEAN: return constant_instruction_debug("OP_BOOLEAN", chunk, offset);
        case OP_STRING: return constant_instruction_debug("OP_STRING", chunk, offset);
        case OP_PRINT: return simple_instruction_debug("OP_PRINT", chunk, offset);
        default:
            fatal_printf("Undefined instruction!\n");
    }
    return -1;
}

static inline void print_instruction_debug(const char* name, size_t offset){
    printf("%04X | %s", (unsigned)offset, name);
}

static inline size_t simple_instruction_debug(const char* name, const struct bytecode_chunk* chunk, size_t offset){
    print_instruction_debug(name, offset);
    printf(" 0x%02X\n", chunk->_code.data[0]);
    return offset+1;
}
static inline size_t constant_instruction_debug(const char* name, const struct bytecode_chunk* chunk, size_t offset){
    #define EXTRACTED_VALUE() ((*(union _inner_value_t*)(chunk->_data.data + (*(int*)(chunk->_code.data + offset + 1)))))

    print_instruction_debug(name, offset);
    switch (*chunk->_code.data) {
        case OP_NUMBER: printf(" %d\n", EXTRACTED_VALUE().number); break;
        case OP_BOOLEAN: printf(" %s\n", EXTRACTED_VALUE().boolean ? "true" : "false"); break;
        case OP_STRING:{
            if(!IS_OBJSTRING(EXTRACTED_VALUE().obj))
                fatal_printf("constant_instruction_debug(): extracted object is not string\n");
            printf(" \"%s\"\n", ((obj_string_t*)(EXTRACTED_VALUE().obj))->str); break;
        }
        default:
            printf(" Not implemented constant instruction :(\n");
    }
    return offset+5;
    #undef EXTRACTED_VALUE
}

void bcchunk_disassemble(const char* chunk_name, const struct bytecode_chunk* chunk){
    printf("=== Disassemble of %s chunk ===\n", chunk_name);
    for(size_t offset = 0; offset < chunk->_code.size;)
        offset = instruction_debug(chunk, offset);
}

void bcchunk_write_simple_op(struct bytecode_chunk* chunk, op_t op){
    bcchunk_write_code(chunk, op);
}

//8 bytes in the instruction
void bcchunk_write_value(struct bytecode_chunk* chunk, value_t data){
    if(IS_NUMBER(data))
        bcchunk_write_code(chunk, OP_NUMBER);
    else if(IS_BOOLEAN(data))
        bcchunk_write_code(chunk, OP_BOOLEAN);
    else if(IS_OBJ(data))
        switch (AS_OBJ(data)->type) {
            case OBJ_STRING: bcchunk_write_code(chunk, OP_STRING); break;
            default: fatal_printf("Undefined obj_type on bcchunk_write_value()\n");
        }
    else 
        fatal_printf("Not implemented instruction :(\n");
    chunk_write_number(&chunk->_code, chunk->_data.size);
    chunk_write_value(&chunk->_data, data);
}

void bcchunk_generate(const ast_node* root, struct bytecode_chunk* chunk){
    switch (root->type) {
        case AST_PRINT: 
            parse_ast_bin_expr(root->data.ptr, chunk);
            bcchunk_write_code(chunk, OP_PRINT);
            break;
        default:
            fatal_printf("Statement root expected in bcchunk_generate()!\n");
    }
    bcchunk_write_simple_op(chunk, OP_RETURN);
}

static void parse_ast_bin_expr(ast_node* node, struct bytecode_chunk* chunk){
    #define BIN_OP(op) do {\
        parse_ast_bin_expr(((struct ast_binary*)node->data.ptr)->left, chunk);\
        parse_ast_bin_expr(((struct ast_binary*)node->data.ptr)->right, chunk);\
        bcchunk_write_simple_op(chunk, op);\
    }while(0)

    switch (node->type) {
        case AST_ADD: 
            BIN_OP(OP_ADD);
            break;
        case AST_SUB: 
            BIN_OP(OP_SUB);
            break;
        case AST_MUL: 
            BIN_OP(OP_MUL);
            break;
        case AST_DIV: 
            BIN_OP(OP_DIV);
            break;
        case AST_AND:
            BIN_OP(OP_AND);
            break;
        case AST_OR: 
            BIN_OP(OP_OR);
            break;
        case AST_XOR:   
            BIN_OP(OP_XOR);
            break;
        case AST_NOT:{
            parse_ast_bin_expr(node->data.ptr, chunk);
            bcchunk_write_simple_op(chunk, OP_NOT);
            break;
        }
        case AST_NUMBER: case AST_BOOLEAN:case AST_STRING:
            bcchunk_write_value(chunk, node->data.val);
            break;
        default:
            fatal_printf("Expected expression in parse_ast_bin_expr()!\n");
    }

    #undef BIN_OP
}

static inline value_t readvalue(const struct bytecode_chunk* chunk, size_t code_offset){
    #define READ_CONSTANT_INDEX() (*(int*)(chunk->_code.data + code_offset + 1))
    #define READ_INSTRUCTION_CONSTANT(type) ( ((type*)chunk->_data.data) [READ_CONSTANT_INDEX()] )

    value_t res;
    switch (chunk->_code.data[code_offset]) {
        case OP_NUMBER:
            res.type = VT_NUMBER;
            res.as = INNERVALUE_AS_NUMBER(READ_INSTRUCTION_CONSTANT(int));
            break; 
        case OP_BOOLEAN:
            res.type = VT_BOOL;
            res.as = INNERVALUE_AS_BOOLEAN(READ_INSTRUCTION_CONSTANT(bool));
            break;
        case OP_STRING:
            res.type = VT_OBJ;
            res.as = INNERVALUE_AS_OBJ(READ_INSTRUCTION_CONSTANT(obj_t*));
            break;
        default:
        fatal_printf("Undefined operation in readvalue()!\n");
    }

    #undef READ_CONSTANT_INDEX
    #undef READ_INSTRUCTION_CONSTANT
    return res;
}