#include "bytecode.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

static inline void chunk_realloc(struct chunk* chunk, size_t newsize);

static inline vm_word_t instruction_debug(const struct bytecode_chunk* chunk, vm_word_t ip);
static inline void print_instruction_debug(const char* name, vm_word_t offset);
static inline vm_word_t simple_instruction_debug(const char* name, vm_word_t offset);
static inline vm_word_t constant_instruction_debug(const char* name, vm_word_t offset, byte_t constant);

void chunk_init(struct chunk* chunk){
    chunk->size = 0;
    chunk->capacity = CHUNK_BASE_CAPACITY;
    if((chunk->data = malloc(CHUNK_BASE_CAPACITY)) == NULL)
        fatal_printf("malloc() returned NULL!\n");
}
void chunk_write(struct chunk* chunk, byte_t byte){
    if (chunk->capacity <= chunk->size)
        chunk_realloc(chunk, 1 << chunk->size);
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

void bcchunk_write_code(struct bytecode_chunk* chunk, byte_t byte){
    chunk_write(&chunk->_code, byte);
}

void bcchunk_write_data(struct bytecode_chunk* chunk, byte_t byte){
    chunk_write(&chunk->_data, byte);
}

static inline vm_word_t instruction_debug(const struct bytecode_chunk* chunk, vm_word_t ip){
    switch (chunk->_code.data[ip]) {
        case OP_RETURN: return simple_instruction_debug("OP_RETURN", ip);
        case OP_CONSTANT: return constant_instruction_debug("OP_CONSTANT", ip, chunk->_data.data[chunk->_code.data[ip+1]]);
        default:
            fatal_printf("Undefined instruction!\n");
    }
    return -1;
}

static inline void print_instruction_debug(const char* name, vm_word_t offset){
    printf("%04X | %s", offset, name);
}

static inline vm_word_t simple_instruction_debug(const char* name, vm_word_t offset){
    print_instruction_debug(name, offset);
    putchar('\n');
    return offset+1;
}
static inline vm_word_t constant_instruction_debug(const char* name, vm_word_t offset, byte_t constant){
    print_instruction_debug(name, offset);
    printf(" %d\n", constant);
    return offset+2;
}

void bcchunk_disassemble(const char* chunk_name, const struct bytecode_chunk* chunk){
    printf("Disassemble of %s chunk:\n", chunk_name);
    for(vm_word_t offset = 0 ;offset < chunk->_code.size;)
        offset = instruction_debug(chunk, offset);
}