#ifndef BYTECODE_H
#define BYTECODE_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define CHUNK_BASE_CAPACITY (1024)

struct ast_node;

typedef uint8_t byte_t;

typedef enum{
    VT_BOOL,
    VT_NUMBER,
    VT_STRING
}value_type;

union _inner_value_t{
    bool boolean;
    int number;
    char* str;
};

#define INNERVALUE_AS_NUMBER(value) ((union _inner_value_t){.number = (value)})
#define INNERVALUE_AS_BOOLEAN(value) ((union _inner_value_t){.boolean = (value)})
#define INNERVALUE_AS_STRING(value) ((union _inner_value_t){.str = (value)})

typedef struct{
    value_type type;
    union _inner_value_t as;
}value_t;

#define VALUE_BOOLEAN(value) ((value_t){.type = VT_BOOL, .as = {.boolean = (value)}})
#define VALUE_NUMBER(value) ((value_t){.type = VT_NUMBER, .as = {.number = (value)}})
#define VALUE_STRING(value) ((value_t){.type = VT_STRING, .as = {.str = (value)}})

#define AS_NUMBER(value) ((value).as.number)
#define AS_BOOLEAN(value) ((value).as.boolean)
#define AS_STRING(value) ((value).as.str)

#define IS_BOOLEAN(value) ((value).type == VT_BOOL)
#define IS_NUMBER(value) ((value).type == VT_NUMBER)
#define IS_STRING(value) ((value).type == VT_STRING)

typedef enum{
    //end execution
    OP_RETURN,
    //get value from the stack and print it
    OP_PRINT,
    //read next 3 bytes and push them to the stack
    OP_NUMBER,
    OP_BOOLEAN,
    OP_STRING,
    //binary ops
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    
    OP_AND,
    OP_OR,
    OP_XOR,
    OP_NOT
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
void chunk_write_number(struct chunk* chunk, int number);
void chunk_write_value(struct chunk* chunk, value_t val);
void chunk_free(struct chunk* chunk);

//initialize chunks with base capacity
void bcchunk_init(struct bytecode_chunk* chunk);
void bcchunk_free(struct bytecode_chunk* chunk);
void bcchunk_write_simple_op(struct bytecode_chunk* chunk, op_t op);

void bcchunk_write_value(struct bytecode_chunk* chunk, value_t data);

//for debug purposes
void bcchunk_disassemble(const char* chunk_name, const struct bytecode_chunk* chunk);

void bcchunk_generate(const struct ast_node* root, struct bytecode_chunk* chunk);

#endif