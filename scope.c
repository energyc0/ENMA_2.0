#include "scope.h"
#include "bytecode.h"
#include "cycler.h"
#include "lang_types.h"
#include "symtable.h"
#include "utils.h"
#include "ast.h"
#include "hash_table.h"
#include <string.h>

struct scope _scope = {
    .locals = {},
    .locals_count = 0,
    .arguments = {},
    .arguments_count = 0,
    .current_depth = 0,
    .current_class = NULL,
    .is_constructor = false
};

//push it on the 'locals' stack
//mark as undefined (depth = -1)
static void declare_local(obj_id_t* id);

//mark 'top' local as defined by defining its depth
static void define_local();

//check if the variable has already defined in this scope
//return true if it is defined
static bool check_current_depth_local(obj_string_t* id);
static inline void pop_locals(struct bytecode_chunk* chunk, int var_k);
static void perform_local_global_op(struct bytecode_chunk* chunk, const obj_id_t* id, op_t local, op_t global, int line);
static bool resolve_field(struct bytecode_chunk* chunk, const obj_id_t* id, int line, op_t op);

static int find_argument(const obj_id_t* id);

void scope_init(){
    //garbage collector stores this memory
    _scope.this_ = mk_objid("this", strlen("this"), hash_string("this", strlen("this")));
    symtable_set(_scope.this_, VALUE_UNINIT);
}

void begin_scope(){
    _scope.current_depth++;
}

void begin_cycle(struct bytecode_chunk* chunk){
    start_parse_cycle(chunk);
    begin_scope();
}

bool is_global_scope(){
    return _scope.current_depth == 0;
}

int get_scope(){
    return _scope.current_depth;
}

void scope_set_class(obj_class_t* cl){
    _scope.current_class = cl;
}

obj_class_t* scope_get_class(){
    return _scope.current_class;
}

void end_cycle(struct bytecode_chunk* chunk){
    end_parse_cycle(chunk);
    end_scope(chunk);
}

void end_scope(struct bytecode_chunk* chunk){
    if(--_scope.current_depth < 0)
        compile_error_printf("Extraneous closing brace ('}')\n");
    if(_scope.current_depth == 0)
        _scope.arguments_count = 0;
    
    int var_k = 0;
    for(;_scope.locals_count > 0 && _scope.locals[_scope.locals_count-1].depth > _scope.current_depth; _scope.locals_count--, var_k++);
    pop_locals(chunk, var_k);
}

int count_scope_vars(){
    int res = 0;
    for(int i = _scope.locals_count - 1;i >= 0 && _scope.locals[i].depth == _scope.current_depth; i--){
        res++;
    }
    return res;
}

bool declare_variable(obj_id_t* id){
    if(check_current_depth_local(id))
        return false;
    if(!is_global_scope()){
        declare_local(id);
    }
    //global variable is already in the symtable
    //it is marked as VT_NULL value
    return true;
}

bool define_variable(obj_id_t* id, struct ast_node* expr, struct bytecode_chunk* chunk){
    if(is_global_scope()){
        value_t val;
        if(!symtable_get(id, &val))
            compile_error_printf("Identifier '%s' is not in symtable! Some shit has occured!\n", id->str);
        if(!IS_NONE(val))
            compile_error_printf("'%s' variable redefenition.\n", id->str);
        symtable_set(id, ast_eval(expr));
    }else{
        bcchunk_write_expression(expr, chunk, line_counter);
        define_local();
        //local is just on the stack
        //bcchunk_write_simple_op(chunk, OP_DEFINE_LOCAL, line_counter);
    }
    return true;
}

static inline void pop_locals(struct bytecode_chunk* chunk, int var_k){
    if(var_k == 1){
        bcchunk_write_simple_op(chunk, OP_POP, line_counter);
    }else if(var_k > 1){
        bcchunk_write_simple_op(chunk, OP_POPN, line_counter);
        bcchunk_write_constant(chunk, var_k, line_counter);
    }
}

static bool check_current_depth_local(obj_string_t* id){
    for(int i = _scope.locals_count - 1;i >= 0 && _scope.locals[i].depth == _scope.current_depth; i--){
        if(is_equal_objstring(id, _scope.locals[i].id))
            return true;
    }
    return false;
}

static void declare_local(obj_id_t* id){
    if(_scope.locals_count >= LOCALS_COUNT)
        compile_error_printf("Locals number has reached limit!\n");
    _scope.locals[_scope.locals_count].depth = -1;
    _scope.locals[_scope.locals_count].id = id;
    ++_scope.locals_count;
}

static void define_local(){
    _scope.locals[_scope.locals_count-1].depth = _scope.current_depth;
}

void scope_constructor_start(){
    begin_scope();
    _scope.is_constructor = true;
}
void scope_constructor_end(struct bytecode_chunk* chunk){
    end_scope(chunk);
    _scope.is_constructor = false;
}
bool scope_is_constructor(){
    return _scope.is_constructor;
}
void scope_add_instance_data(struct bytecode_chunk* chunk){
    declare_local(_scope.this_); //this argument is an instance that uses this method
    define_local();
    bcchunk_write_simple_op(chunk, OP_INSTANCE, line_counter);
    bcchunk_write_value(chunk, VALUE_OBJ(_scope.current_class), line_counter);
}

obj_id_t* scope_get_this(){
    return _scope.this_;
}

bool declare_argument(obj_id_t* id){
    if(find_argument(id) != -1)
        return false;
    if(_scope.arguments_count >= ARGUMENTS_COUNT)
        compile_error_printf("Arguments number has reached limit!\n");
    _scope.arguments[_scope.arguments_count++] = id;
    return true;
}

int resolve_local(const obj_id_t* id){
    int idx = find_argument(id);
    if(idx != -1)
        return -idx - 3;
    for(int i = _scope.locals_count - 1; i >= 0; i--)
        if(is_equal_objstring(_scope.locals[i].id, id)){
            if(_scope.locals[i].depth == -1)
                compile_error_printf("Cannot use as operand currently defining variable\n");
            return i;
        }
    return -1;
}

static void perform_local_global_op(struct bytecode_chunk* chunk, const obj_id_t* id, op_t local, op_t global, int line){
    int idx;
    if(!is_global_scope() && (idx = resolve_local(id)) != -1){
            bcchunk_write_simple_op(chunk, local, line);
            bcchunk_write_value(chunk, VALUE_NUMBER(idx), line);
    }else{
        bcchunk_write_simple_op(chunk, global, line);
        bcchunk_write_value(chunk, VALUE_OBJ(id), line);
    }
}

void write_set_var(struct bytecode_chunk* chunk, const obj_id_t* id, int line){
    if(!resolve_field(chunk, id, line, OP_SET_PROPERTY))
        perform_local_global_op(chunk, id, OP_SET_LOCAL, OP_SET_GLOBAL, line);
}

void write_get_var(struct bytecode_chunk* chunk, const obj_id_t* id, int line){
    if(!resolve_field(chunk, id, line, OP_GET_PROPERTY))
        perform_local_global_op(chunk, id, OP_GET_LOCAL, OP_GET_GLOBAL, line);
}

static bool resolve_field(struct bytecode_chunk* chunk, const obj_id_t* id, int line, op_t op){
    if(_scope.current_class == NULL)
        return false;

    if(table_check(_scope.current_class->properties, id, NULL)){
        write_get_var(chunk, _scope.this_, line);
        bcchunk_write_simple_op(chunk, op, line);
        bcchunk_write_value(chunk, VALUE_OBJ(id), line);
        return true;
    }

    return false;
}

void write_postincr_var(struct bytecode_chunk* chunk, const obj_id_t* id, int line){
    perform_local_global_op(chunk, id, OP_POSTINCR_LOCAL, OP_POSTINCR_GLOBAL, line);
}
void write_prefincr_var(struct bytecode_chunk* chunk, const obj_id_t* id, int line){
    perform_local_global_op(chunk, id, OP_PREFINCR_LOCAL, OP_PREFINCR_GLOBAL, line);
}
void write_postdecr_var(struct bytecode_chunk* chunk, const obj_id_t* id, int line){
    perform_local_global_op(chunk, id, OP_POSTDECR_LOCAL, OP_POSTDECR_GLOBAL, line);
}
void write_prefdecr_var(struct bytecode_chunk* chunk, const obj_id_t* id, int line){
    perform_local_global_op(chunk, id, OP_PREFDECR_LOCAL, OP_PREFDECR_GLOBAL, line);
}

static int find_argument(const obj_id_t* id){
    for(int i = 0; i < _scope.arguments_count; i++)
        if(id == _scope.arguments[i])
            return i;
    return -1;
}