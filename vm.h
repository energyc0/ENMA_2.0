#ifndef VM_H
#define VM_H

#include <stdint.h>
#include <stdio.h>
#include "bytecode.h"

#define STACK_SIZE (256)

struct virtual_machine{
    struct bytecode_chunk* code;
    byte_t* ip;
    value_t stack[STACK_SIZE];
    value_t* stack_top;
    value_t* bp;
};

typedef enum{
    VME_SUCCESS,
    VME_RUNTIME_ERROR,
    VME_COMPILE_ERROR
} vm_execute_result;

void vm_interpret();

#endif