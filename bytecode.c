#include "bytecode.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "lang_types.h"
#include "scope.h"
#include "utils.h"

static void chunk_init(struct chunk* chunk);
static void chunk_write(struct chunk* chunk, byte_t byte);
static void chunk_write_number(struct chunk* chunk, int number);
static void chunk_write_value(struct chunk* chunk, value_t val);
static void chunk_free(struct chunk* chunk);
static inline void chunk_realloc(struct chunk* chunk, size_t newsize);

static inline void bcchunk_write_code(struct bytecode_chunk* chunk, byte_t byte, int line);
static inline void bcchunk_write_data(struct bytecode_chunk* chunk, byte_t byte);

static inline size_t instruction_debug(const struct bytecode_chunk* chunk, size_t offset);
static inline void print_instruction_debug(const char* name, const struct bytecode_chunk* chunk, size_t offset);
static inline size_t simple_instruction_debug(const char* name, const struct bytecode_chunk* chunk, size_t offset);
static inline size_t constant_instruction_debug(const char* name, const struct bytecode_chunk* chunk, size_t offset);

static void parse_ast_bin_expr(const ast_node* node, struct bytecode_chunk* chunk, int line);
static inline value_t readvalue(const struct bytecode_chunk* chunk, size_t code_offset);

static void chunk_init(struct chunk* chunk){
    chunk->size = 0;
    chunk->capacity = CHUNK_BASE_CAPACITY;
    chunk->data = emalloc(sizeof(chunk->data[0]) * CHUNK_BASE_CAPACITY);
}

static void chunk_free(struct chunk* chunk){
    chunk->size = 0;
    chunk->capacity = 0;
    free(chunk->data);
    chunk->data = NULL;
}

static void chunk_write(struct chunk* chunk, byte_t byte){
    if (chunk->capacity <= chunk->size)
        chunk_realloc(chunk, chunk->capacity + CHUNK_BASE_CAPACITY);
    chunk->data[chunk->size++] = byte;
}

static void chunk_write_number(struct chunk* chunk, int number){
    if (chunk->capacity < chunk->size + sizeof(number))
        chunk_realloc(chunk, chunk->capacity + CHUNK_BASE_CAPACITY);
    *((int*)(chunk->data + chunk->size)) = number;
    chunk->size += sizeof(number);
}

static void chunk_write_value(struct chunk* chunk, value_t val){
    if (chunk->capacity < chunk->size + sizeof(val.as))
        chunk_realloc(chunk, chunk->capacity + CHUNK_BASE_CAPACITY);
    *((union _inner_value_t*)(chunk->data + chunk->size)) = val.as;
    chunk->size += sizeof(val.as);
}

static inline void chunk_realloc(struct chunk* chunk, size_t newsize){
    if((chunk->capacity = newsize) == 0)
        fatal_printf("chunk_realloc(): newsize = 0!\n");
    chunk->data = erealloc(chunk->data, newsize);
    chunk->size = chunk->size > newsize ? chunk->size : newsize;
}

void bcchunk_init(struct bytecode_chunk* chunk){
    chunk_init(&chunk->_code);
    chunk_init(&chunk->_data);
    chunk_init(&chunk->_line_data);
}

void bcchunk_free(struct bytecode_chunk* chunk){
    chunk_free(&chunk->_code);
    chunk_free(&chunk->_data);
    chunk_free(&chunk->_line_data);
}

static inline void bcchunk_write_code(struct bytecode_chunk* chunk, byte_t byte, int line){
    chunk_write(&chunk->_code, byte);
    chunk_write_number(&chunk->_line_data, line);
}


static inline void bcchunk_write_data(struct bytecode_chunk* chunk, byte_t byte){
    chunk_write(&chunk->_data, byte);
}

static inline size_t instruction_debug(const struct bytecode_chunk* chunk, size_t offset){
    op_t op = chunk->_code.data[offset];
    switch (op) {
        case OP_RETURN: return simple_instruction_debug(op_to_string(op),chunk, offset);
        case OP_ADD: return simple_instruction_debug(op_to_string(op),chunk, offset);
        case OP_SUB: return simple_instruction_debug(op_to_string(op),chunk, offset);
        case OP_DIV: return simple_instruction_debug(op_to_string(op),chunk, offset);
        case OP_MUL: return simple_instruction_debug(op_to_string(op),chunk, offset);
        case OP_AND: return simple_instruction_debug(op_to_string(op),chunk, offset);
        case OP_OR: return simple_instruction_debug(op_to_string(op),chunk, offset);
        case OP_XOR: return simple_instruction_debug(op_to_string(op),chunk, offset);
        case OP_NOT: return simple_instruction_debug(op_to_string(op),chunk, offset);
        case OP_EQUAL: return simple_instruction_debug(op_to_string(op),chunk, offset);
        case OP_GREATER: return simple_instruction_debug(op_to_string(op),chunk, offset);
        case OP_LESS:  return simple_instruction_debug(op_to_string(op),chunk, offset);
        case OP_NUMBER: return constant_instruction_debug(op_to_string(op),chunk, offset);
        case OP_BOOLEAN: return constant_instruction_debug(op_to_string(op), chunk, offset);
        case OP_STRING: return constant_instruction_debug(op_to_string(op), chunk, offset);
        case OP_DEFINE_GLOBAL: return constant_instruction_debug(op_to_string(op), chunk, offset);
        case OP_GET_GLOBAL: return constant_instruction_debug(op_to_string(op), chunk, offset);
        case OP_SET_GLOBAL: return constant_instruction_debug(op_to_string(op), chunk, offset);
        case OP_DEFINE_LOCAL: return constant_instruction_debug(op_to_string(op), chunk, offset);
        case OP_GET_LOCAL: return constant_instruction_debug(op_to_string(op), chunk, offset);
        case OP_SET_LOCAL: return constant_instruction_debug(op_to_string(op), chunk, offset);
        case OP_PRINT: return simple_instruction_debug(op_to_string(op), chunk, offset);
        case OP_POP: return simple_instruction_debug(op_to_string(op), chunk, offset);
        case OP_POPN: return constant_instruction_debug(op_to_string(op), chunk, offset);
        case OP_PUSH_BP: return simple_instruction_debug(op_to_string(op), chunk, offset);
        case OP_POP_BP: return simple_instruction_debug(op_to_string(op), chunk, offset);
        case OP_BP_AS_SP: return simple_instruction_debug(op_to_string(op), chunk, offset);
        case OP_SP_AS_BP: return simple_instruction_debug(op_to_string(op), chunk, offset);
        case OP_JUMP: return constant_instruction_debug(op_to_string(op), chunk, offset);
        case OP_FJUMP: return constant_instruction_debug(op_to_string(op), chunk, offset);
        default:
            fatal_printf("Undefined instruction! Check instruction_debug().\n");
    }
    return -1;
}

static inline void print_instruction_debug(const char* name, const struct bytecode_chunk* chunk, size_t offset){
    printf("%10d | %04X | %s",((int*)chunk->_line_data.data)[offset], (unsigned)offset, name);
}

static inline size_t simple_instruction_debug(const char* name, const struct bytecode_chunk* chunk, size_t offset){
    print_instruction_debug(name,chunk, offset);
    printf(" 0x%02X\n", chunk->_code.data[offset]);
    return offset+1;
}
static inline size_t constant_instruction_debug(const char* name, const struct bytecode_chunk* chunk, size_t offset){
    #define EXTRACTED_VALUE (((union _inner_value_t*)(chunk->_data.data + (*(int*)(chunk->_code.data + offset + 1)))))

    union _inner_value_t* extracted_value = EXTRACTED_VALUE;
    print_instruction_debug(name,chunk, offset);
    switch (*(chunk->_code.data + offset)) {
        case OP_NUMBER:
            printf(" [0x%X]\n", extracted_value->number);
            break;
        case OP_BOOLEAN:
            printf(" [%s]\n", extracted_value->boolean ? "true" : "false");
            break;
        case OP_STRING:{
            if(!IS_OBJSTRING(extracted_value->obj))
                fatal_printf("constant_instruction_debug(): extracted object is not string\n");
            printf(" [\"%s\"] %p\n", ((obj_string_t*)(extracted_value->obj))->str,((obj_string_t*)(extracted_value->obj))->str);
            break;
        }
        case OP_GET_GLOBAL: case OP_SET_GLOBAL:case OP_DEFINE_GLOBAL:{
            if(!IS_OBJIDENTIFIER(extracted_value->obj))
                fatal_printf("constant_instruction_debug(): extracted object is not identifier\n");
            printf(" [\"%s\"] %p\n", ((obj_id_t*)(extracted_value->obj))->str,((obj_id_t*)(extracted_value->obj))->str);
            break;
        }
        case OP_GET_LOCAL: case OP_SET_LOCAL: case OP_DEFINE_LOCAL:{
            printf(" stack index: %d\n", extracted_value->number);
            break;
        }
        case OP_POPN:case OP_JUMP: case OP_FJUMP:{
            printf(" [0x%X]\n", *(int*)(chunk->_code.data + offset + 1));
            break;
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

void bcchunk_write_simple_op(struct bytecode_chunk* chunk, op_t op, int line){
    bcchunk_write_code(chunk, op, line);
}

int bcchunk_get_codesize(struct bytecode_chunk* chunk){
    return chunk->_code.size;
}

void bcchunk_write_constant(struct bytecode_chunk* chunk, int num, int line){
    chunk_write_number(&chunk->_code, num);
    for(size_t i  = 0; i < sizeof(int); i++)
        chunk_write_number(&chunk->_line_data, line);
}

void bcchunk_rewrite_constant(struct bytecode_chunk *chunk, int offset, int num){
    *((int*)(chunk->_code.data + offset)) = num;
}

//5 bytes in the instruction
void bcchunk_write_value(struct bytecode_chunk* chunk, value_t data, int line){
    chunk_write_number(&chunk->_code, chunk->_data.size);
    for(size_t i  = 0; i < sizeof(int); i++)
        chunk_write_number(&chunk->_line_data, line);
    chunk_write_value(&chunk->_data, data);
}

void bcchunk_write_expression(const ast_node* root, struct bytecode_chunk* chunk, int line){
#ifdef DEBUG
    ast_debug_tree(root);
#endif
    parse_ast_bin_expr(root, chunk, line);
}

static void parse_ast_bin_expr(const ast_node* node, struct bytecode_chunk* chunk, int line){
    #define BIN_OP(op) do {\
        parse_ast_bin_expr(((struct ast_binary*)node->data.ptr)->left, chunk, line);\
        parse_ast_bin_expr(((struct ast_binary*)node->data.ptr)->right, chunk, line);\
        bcchunk_write_simple_op(chunk, op, line);\
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
        case AST_EQUAL:
            BIN_OP(OP_EQUAL);
            break;
        case AST_NEQUAL:
            BIN_OP(OP_EQUAL);
            bcchunk_write_simple_op(chunk, OP_NOT, line);
            break;
        case AST_EGREATER:
            BIN_OP(OP_LESS);
            bcchunk_write_simple_op(chunk, OP_NOT,line); 
            break;
        case AST_GREATER:
            BIN_OP(OP_GREATER);
            break;
        case AST_ELESS:
            BIN_OP(OP_GREATER);
            bcchunk_write_simple_op(chunk, OP_NOT, line); 
            break;
        case AST_LESS:
            BIN_OP(OP_LESS);
            break;
        case AST_ASSIGN:{
            struct ast_binary* temp = ((struct ast_binary*)node->data.ptr);
            if(temp->left->type != AST_IDENT)
                interpret_error_printf(line, "Left operand is not assignable!\n");
            parse_ast_bin_expr(temp->right, chunk, line);
            write_set_var(chunk, temp->left, line);
            break;
        }
        case AST_NOT:{
            parse_ast_bin_expr(node->data.ptr, chunk, line);
            bcchunk_write_simple_op(chunk, OP_NOT, line);
            break;
        }
        case AST_NUMBER:
            bcchunk_write_code(chunk, OP_NUMBER, line);
            bcchunk_write_value(chunk, node->data.val, line);
            break;
        case AST_BOOLEAN:
            bcchunk_write_code(chunk, OP_BOOLEAN, line);
            bcchunk_write_value(chunk, node->data.val, line);
            break;
        case AST_STRING:
            bcchunk_write_code(chunk, OP_STRING, line);
            bcchunk_write_value(chunk, node->data.val, line);
            break;
        case AST_IDENT:
            write_get_var(chunk, node, line);
            break;
        default:
            fatal_printf("Expected expression in parse_ast_bin_expr()!\n Node type is %d\n", node->type);
    }

    #undef BIN_OP
}

const char* op_to_string(op_t op){
    static const char* ops[] = {
        [OP_RETURN] = "OP_RETURN",
        [OP_ADD] = "OP_ADD",
        [OP_SUB] = "OP_SUB",
        [OP_DIV] = "OP_DIV",
        [OP_MUL] = "OP_MUL",
        [OP_AND] = "OP_AND",
        [OP_OR] = "OP_OR",
        [OP_XOR] = "OP_XOR",
        [OP_NOT] = "OP_NOT",
        [OP_EQUAL] = "OP_EQUAL",
        [OP_GREATER] = "OP_GREATER",
        [OP_LESS] = "OP_LESS",
        [OP_NUMBER] = "OP_NUMBER",
        [OP_BOOLEAN] = "OP_BOOLEAN",
        [OP_STRING] = "OP_STRING",
        [OP_DEFINE_GLOBAL] = "OP_DEFINE_GLOBAL",
        [OP_GET_GLOBAL] = "OP_GET_GLOBAL",
        [OP_SET_GLOBAL] = "OP_SET_GLOBAL",
        [OP_DEFINE_LOCAL] = "OP_DEFINE_LOCAL",
        [OP_GET_LOCAL] = "OP_GET_LOCAL",
        [OP_SET_LOCAL] = "OP_SET_LOCAL",
        [OP_PRINT] = "OP_PRINT",
        [OP_POP] = "OP_POP",
        [OP_POPN] = "OP_POPN",    
        [OP_PUSH_BP] = "OP_PUSH_BP",
        [OP_POP_BP] = "OP_POP_BP",
        [OP_BP_AS_SP] = "OP_BP_AS_SP",    
        [OP_SP_AS_BP] = "OP_SP_AS_BP",
        [OP_FJUMP] = "OP_FJUMP",
        [OP_JUMP] = "OP_JUMP"
    };
#ifdef DEBUG 
    if(!(0 <= op && op < sizeof(ops) / sizeof(ops[0])))
        fatal_printf("Undefined operation in op_to_string()\n");
#endif
    return ops[op];
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