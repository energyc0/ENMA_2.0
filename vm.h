#ifndef VM_H
#define VM_H

#include <stdint.h>
#include "bytecode.h"

struct virtual_machine{
    struct bytecode_chunk code;
    int32_t ip;
};

//void vm_load_code();
void vm_execute(struct virtual_machine* vm);

#endif