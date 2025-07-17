#ifndef BYTECODE_H
#define BYTECODE_H

#include <stdlib.h>
#include <stdint.h>

#define CHUNK_BASE_CAPACITY (1024)

struct ast_node;

typedef uint8_t byte_t;
typedef int32_t vm_value_t;
typedef int32_t vm_word_t;

typedef enum{
    //end execution
    OP_RETURN,
    //get value from the stack and print it
    OP_PRINT,
    //read next 3 bytes and push them to the stack
    OP_CONSTANT,
    //binary ops
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV
} op_t;

struct chunk{
    size_t capacity;
    size_t size;
    byte_t* data;
};

struct bytecode_chunk{
    struct chunk _code;
    struct chunk _data;
};

//initialize chunk with base capacity
void chunk_init(struct chunk* chunk);
void chunk_write(struct chunk* chunk, byte_t byte);
void chunk_free(struct chunk* chunk);

//initialize chunks with base capacity
void bcchunk_init(struct bytecode_chunk* chunk);
void bcchunk_free(struct bytecode_chunk* chunk);
void bcchunk_write_simple_op(struct bytecode_chunk* chunk, op_t op);
//4 bytes in the instruction
void bcchunk_write_constant(struct bytecode_chunk* chunk, vm_value_t data);

//for debug purposes
void bcchunk_disassemble(const char* chunk_name, const struct bytecode_chunk* chunk);

void bcchunk_generate(const struct ast_node* root, struct bytecode_chunk* chunk);

#endif