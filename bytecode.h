#ifndef BYTECODE_H
#define BYTECODE_H

#include <stdlib.h>
#include <stdint.h>

#define CHUNK_BASE_CAPACITY (1024)

typedef uint8_t byte_t;
typedef int32_t vm_value_t;
typedef uint32_t vm_word_t;
typedef enum{
    OP_RETURN,
    OP_CONSTANT,
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

void chunk_init(struct chunk* chunk);
void chunk_write(struct chunk* chunk, byte_t byte);

void bcchunk_init(struct bytecode_chunk* chunk);
void bcchunk_write_code(struct bytecode_chunk* chunk, byte_t byte);
void bcchunk_write_data(struct bytecode_chunk* chunk, byte_t byte);
void bcchunk_disassemble(const char* chunk_name, const struct bytecode_chunk* chunk);

#endif