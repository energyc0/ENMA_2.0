#include "scope.h"
#include "bytecode.h"
#include "lang_types.h"
#include "symtable.h"
#include "utils.h"
#include "ast.h"

struct scope _scope;

//push it on the 'locals' stack
//mark as undefined (depth = -1)
static void declare_local(obj_id_t* id);

//mark 'top' local as defined by defining its depth
static void define_local();

//check if the variable has already defined in this scope
//return true if it is defined
static bool check_current_depth_local(obj_string_t* id);

void begin_scope(){
    _scope.current_depth++;
}

bool is_global_scope(){
    return _scope.current_depth == 0;
}

void end_scope(){
    if(--_scope.current_depth < 0)
        compile_error_printf("Extraneous closing brace ('}')\n");
    for(;_scope.locals_count > 0 && _scope.locals[_scope.locals_count-1].depth > _scope.current_depth; _scope.locals_count--);
}

bool is_variable_exist(obj_id_t* id){
    value_t val;
    symtable_get(id, &val);
    return val.type != VT_NULL;
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
    bcchunk_write_expression(expr, chunk, line_counter);
    if(is_global_scope()){
        bcchunk_write_simple_op(chunk, OP_DEFINE_GLOBAL, line_counter);
        bcchunk_write_value(chunk, VALUE_OBJ(id), line_counter);
    }else{
        define_local();
        //local is just on the stack
        //bcchunk_write_simple_op(chunk, OP_DEFINE_LOCAL, line_counter);
    }
    return true;
}

static bool check_current_depth_local(obj_string_t* id){
    for(int i = _scope.locals_count - 1;i >= 0 && _scope.locals[i].depth == _scope.current_depth; i--){
        if(is_equal_objstring(id, _scope.locals[i].id))
            return true;
    }
    return false;
}

static void declare_local(obj_id_t* id){
    _scope.locals[_scope.locals_count].depth = -1;
    _scope.locals[_scope.locals_count].id = id;
    ++_scope.locals_count;
}

static void define_local(){
    _scope.locals[_scope.locals_count-1].depth = _scope.current_depth;
}

int resolve_local(obj_id_t* id){
    for(int i = _scope.locals_count - 1; i >= 0; i--)
        if(is_equal_objstring(_scope.locals[i].id, id)){
            if(_scope.locals[i].depth == -1)
                compile_error_printf("Cannot use as operand currently defining variable\n");
            return i;
        }
    return -1;
}

void write_set_var(struct bytecode_chunk* chunk, const struct ast_node* id_node, int line){
    if(is_global_scope()){
        bcchunk_write_simple_op(chunk, OP_SET_GLOBAL, line);
        bcchunk_write_value(chunk, id_node->data.val, line);
    }else{
        int idx = resolve_local(AS_OBJIDENTIFIER(id_node->data.val));
        if(idx == -1)
            interpret_error_printf(line, "Undefined identifer '%s'\n", AS_OBJIDENTIFIER(id_node->data.val)->str);
        bcchunk_write_simple_op(chunk, OP_SET_LOCAL, line);
        bcchunk_write_value(chunk, VALUE_NUMBER(idx), line);
    }
}

void write_get_var(struct bytecode_chunk* chunk, const struct ast_node* id_node, int line){
    if(is_global_scope()){
        bcchunk_write_simple_op(chunk, OP_GET_GLOBAL, line);
        bcchunk_write_value(chunk, id_node->data.val, line);
    }else{
        int idx = resolve_local(AS_OBJIDENTIFIER(id_node->data.val));
        if(idx == -1)
            interpret_error_printf(line, "Undefined identifer '%s'\n", AS_OBJIDENTIFIER(id_node->data.val)->str);
        bcchunk_write_simple_op(chunk, OP_GET_LOCAL, line);
        bcchunk_write_value(chunk, VALUE_NUMBER(idx), line);
    }
}