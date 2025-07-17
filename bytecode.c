#include "bytecode.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "token.h"
#include "utils.h"

static inline void chunk_realloc(struct chunk* chunk, size_t newsize);

static inline void bcchunk_write_code(struct bytecode_chunk* chunk, byte_t byte);
static inline void bcchunk_write_data(struct bytecode_chunk* chunk, byte_t byte);

static inline size_t instruction_debug(const struct bytecode_chunk* chunk, size_t offset);
static inline void print_instruction_debug(const char* name, size_t offset);
static inline size_t simple_instruction_debug(const char* name, const byte_t* code, size_t offset);
static inline size_t constant_instruction_debug(const char* name, const byte_t* code, size_t offset);

static void parse_ast_bin_expr(ast_node* node, struct bytecode_chunk* chunk);

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
        case OP_RETURN: return simple_instruction_debug("OP_RETURN",chunk->_code.data, offset);
        case OP_ADD: return simple_instruction_debug("OP_ADD",chunk->_code.data, offset);
        case OP_SUB: return simple_instruction_debug("OP_SUB",chunk->_code.data, offset);
        case OP_DIV: return simple_instruction_debug("OP_DIV",chunk->_code.data, offset);
        case OP_MUL: return simple_instruction_debug("OP_MUL",chunk->_code.data, offset);
        case OP_CONSTANT: return constant_instruction_debug("OP_CONSTANT",chunk->_code.data, offset);
        case OP_PRINT: return simple_instruction_debug("OP_PRINT", chunk->_code.data, offset);
        default:
            fatal_printf("Undefined instruction!\n");
    }
    return -1;
}

static inline void print_instruction_debug(const char* name, size_t offset){
    printf("%04X | %s", (unsigned)offset, name);
}

static inline size_t simple_instruction_debug(const char* name, const byte_t* code, size_t offset){
    print_instruction_debug(name, offset);
    printf(" 0x%02X\n", *code);
    return offset+1;
}
static inline size_t constant_instruction_debug(const char* name, const byte_t* code, size_t offset){
    print_instruction_debug(name, offset);
    printf(" %d 0x%08X\n",
         code[offset + 1] + (code[offset + 2] << 8) + (code[offset + 3] << 16) + (code[offset + 4] << 24),
          *(vm_word_t*)code);
    return offset+5;
}

void bcchunk_disassemble(const char* chunk_name, const struct bytecode_chunk* chunk){
    printf("=== Disassemble of %s chunk ===\n", chunk_name);
    for(size_t offset = 0; offset < chunk->_code.size;)
        offset = instruction_debug(chunk, offset);
}

void bcchunk_write_simple_op(struct bytecode_chunk* chunk, op_t op){
    bcchunk_write_code(chunk, op);
}
//4 bytes in the instruction
void bcchunk_write_constant(struct bytecode_chunk* chunk, vm_value_t data){
    bcchunk_write_code(chunk, OP_CONSTANT);
    bcchunk_write_code(chunk, data & 0xFF);
    bcchunk_write_code(chunk, (data >> 8) & 0xFF);
    bcchunk_write_code(chunk, (data >> 16) & 0xFF);
    bcchunk_write_code(chunk, (data >> 24) & 0xFF);
}

void bcchunk_generate(const ast_node* root, struct bytecode_chunk* chunk){
    switch (root->type) {
        case AST_PRINT: 
            parse_ast_bin_expr(root->data, chunk);
            bcchunk_write_code(chunk, OP_PRINT);
            break;
        default:
            fatal_printf("Statement root expected in bcchunk_generate()!\n");
    }
    bcchunk_write_simple_op(chunk, OP_RETURN);
}

static void parse_ast_bin_expr(ast_node* node, struct bytecode_chunk* chunk){
    #define BIN_OP(op) do {\
        parse_ast_bin_expr(((struct ast_binary*)node->data)->left, chunk);\
        parse_ast_bin_expr(((struct ast_binary*)node->data)->right, chunk);\
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
        case AST_CONSTANT:
            bcchunk_write_constant(chunk, *(vm_value_t*)node->data);
            break;
        default:
            fatal_printf("Expected expression in parse_ast_bin_expr()!\n");
    }

    #undef BIN_OP
}