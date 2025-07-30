#ifndef SCOPE_H
#define SCOPE_H

#include <stdbool.h>

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

#endif