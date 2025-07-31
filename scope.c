#include "scope.h"
#include "bytecode.h"
#include "lang_types.h"
#include "symtable.h"
#include "utils.h"

struct scope _scope;

void begin_scope(){
    _scope.current_depth++;
}

bool is_global_scope(){
    return _scope.current_depth == 0;
}

void end_scope(){
    if(--_scope.current_depth < 0)
        compile_error_printf("Extraneous closing brace ('}')\n");

}

bool is_variable_exist(const obj_id_t* id){
    value_t val;
    symtable_get(id, &val);
    return val.type != VT_NULL;
}