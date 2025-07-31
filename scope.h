#ifndef SCOPE_H
#define SCOPE_H

#include "lang_types.h"
#include <stdbool.h>

struct bytecode_chunk;

#define LOCALS_COUNT (256)

struct local{
    char* name;
    int depth;
};

struct scope{
    struct local locals[LOCALS_COUNT];
    int locals_count;
    int current_depth;
};

void begin_scope();
bool is_global_scope();
void end_scope();

bool is_variable_exist(const obj_id_t* id);

#endif