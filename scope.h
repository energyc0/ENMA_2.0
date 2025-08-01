#ifndef SCOPE_H
#define SCOPE_H

#include "lang_types.h"
#include <stdbool.h>

struct bytecode_chunk;
struct ast_node;

#define LOCALS_COUNT (256)

struct local{
    obj_id_t* id;
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

//return count of the variables in the current scope
int count_scope_vars();
//return false if variable exists
bool declare_variable(obj_id_t* id);
bool define_variable(obj_id_t* id, struct ast_node* expr, struct bytecode_chunk* chunk);

//return variable index for vm.stack[]
//return -1 if not found
//print error if try to resolve currently defining variable
int resolve_local(obj_id_t* id);

void write_set_var(struct bytecode_chunk* chunk, const struct ast_node* id_node, int line);
void write_get_var(struct bytecode_chunk* chunk, const struct ast_node* id_node, int line);
#endif