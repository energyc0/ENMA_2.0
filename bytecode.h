#ifndef BYTECODE_H
#define BYTECODE_H

#include "lang_types.h"

#define CHUNK_BASE_CAPACITY (1024)

struct ast_node;

typedef enum{
    //end execution
    OP_RETURN,
    //just pop value out of stack
    OP_POP,
    //pop n values from the stack
    OP_POPN,

    OP_JUMP,
    OP_FJUMP, //jump if a top value on the stack if false

    OP_DEFINE_GLOBAL, //define global in the symtable
    OP_SET_GLOBAL,  //set global in the symtable
    OP_GET_GLOBAL,  //get global from the symtable
    
    OP_DEFINE_LOCAL,    //reserve space in the stack
    OP_SET_LOCAL,       //assign value in the stack
    OP_GET_LOCAL,       //get value from the stack
    //get value from the stack and print it
    OP_PRINT,
    //read (int) index and read value from _data section
    OP_NUMBER,
    OP_BOOLEAN,
    OP_STRING,
    //binary ops
    //pop values from the stack and perform current operation
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    
    OP_AND,
    OP_OR,
    OP_XOR,
    OP_NOT,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    //bp and sp manipulations
    OP_PUSH_BP,
    OP_POP_BP,
    OP_BP_AS_SP,    //BP = SP
    OP_SP_AS_BP     //SP = BP
} op_t;

struct chunk{
    size_t capacity;
    size_t size;
    byte_t* data;
};

struct bytecode_chunk{
    struct chunk _code;
    struct chunk _line_data;
    struct chunk _data;
};

//initialize chunk with base capacity
/*
void chunk_init(struct chunk* chunk);
void chunk_write(struct chunk* chunk, byte_t byte);
void chunk_write_number(struct chunk* chunk, int number);
void chunk_write_value(struct chunk* chunk, value_t val);
void chunk_free(struct chunk* chunk);
*/
const char* op_to_string(op_t op);

//initialize chunks with base capacity
void bcchunk_init(struct bytecode_chunk* chunk);
void bcchunk_free(struct bytecode_chunk* chunk);
int bcchunk_get_codesize(struct bytecode_chunk* chunk);
void bcchunk_write_simple_op(struct bytecode_chunk* chunk, op_t op, int line);
void bcchunk_write_constant(struct bytecode_chunk* chunk, int num, int line);
void bcchunk_rewrite_constant(struct bytecode_chunk* chunk,int offset, int num);
void bcchunk_write_value(struct bytecode_chunk* chunk, value_t data, int line);
void bcchunk_write_expression(const struct ast_node* root, struct bytecode_chunk* chunk, int line);

//for debug purposes
void bcchunk_disassemble(const char* chunk_name, const struct bytecode_chunk* chunk);

#endif