#ifndef SCOPE_H
#define SCOPE_H

#include "bytecode.h"
#include "lang_types.h"
#include <stdbool.h>

struct bytecode_chunk;
struct ast_node;

#define LOCALS_COUNT (256)
#define ARGUMENTS_COUNT (256)

struct local{
    obj_id_t* id;
    int depth;
};

struct scope{
    struct local locals[LOCALS_COUNT];
    int locals_count;

    obj_id_t* arguments[ARGUMENTS_COUNT];
    int arguments_count;

    int current_depth;
};

void begin_scope();
void begin_cycle(struct bytecode_chunk* chunk); //calls begin_scope()

bool is_global_scope();

void end_cycle(struct bytecode_chunk* chunk); //calls end_scope()
void end_scope(struct bytecode_chunk* chunk);


int get_scope();

//return count of the variables in the current scope
int count_scope_vars();

/*return false if variable exists*/
bool declare_variable(obj_id_t* id);
/*return false if variable exists*/
bool define_variable(obj_id_t* id, struct ast_node* expr, struct bytecode_chunk* chunk);
/*return false if variable exists*/
bool declare_argument(obj_id_t* id);

//return variable index for vm.bp[]
//return -1 if not found(vm.bp[-1] is old bp and vm.bp[-2] is return address)
//print error if try to resolve currently defining variable
//resolve locals and arguments
int resolve_local(obj_id_t* id);

void write_set_var(struct bytecode_chunk* chunk, const struct ast_node* id_node, int line);
void write_get_var(struct bytecode_chunk* chunk, const struct ast_node* id_node, int line);
void write_postincr_var(struct bytecode_chunk* chunk, const struct ast_node* id_node, int line);
void write_prefincr_var(struct bytecode_chunk* chunk, const struct ast_node* id_node, int line);
void write_postdecr_var(struct bytecode_chunk* chunk, const struct ast_node* id_node, int line);
void write_prefdecr_var(struct bytecode_chunk* chunk, const struct ast_node* id_node, int line);

#endif