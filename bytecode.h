#ifndef BYTECODE_H
#define BYTECODE_H

#include "lang_types.h"

#define CHUNK_BASE_CAPACITY (1024)

struct ast_node;

typedef enum{
    //end execution or return from function
    OP_RETURN,
    //just pop value out of stack
    OP_POP,
    //pop n values from the stack
    OP_POPN,
    //clear stack from previous call
    //reads constant number
    OP_CLARGS,
    //read next 4 bytes and get obj_function_t* instance
    //place return ip on the stack
    OP_CALL,
    OP_NATIVE_CALL,
    
    OP_JUMP,
    OP_FJUMP, //jump if a top value on the stack if false

    OP_SET_GLOBAL,  //set global in the symtable
    OP_GET_GLOBAL,  //get global from the symtable
    
    OP_SET_LOCAL,       //assign value in the stack
    OP_GET_LOCAL,       //get value from the stack
    //read (int) index and read value from _data section
    OP_NUMBER,
    OP_BOOLEAN,
    OP_STRING,
    OP_NULL, //it is a simple op, just push NULL on the stack
    OP_INSTANCE,
    
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
    /*constant value may be an identifier(for globals) or an index(for locals)*/
    OP_POSTINCR_GLOBAL,
    OP_POSTINCR_LOCAL,
    OP_POSTDECR_GLOBAL,
    OP_POSTDECR_LOCAL,
    OP_PREFINCR_GLOBAL,
    OP_PREFINCR_LOCAL,
    OP_PREFDECR_GLOBAL,
    OP_PREFDECR_LOCAL,
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