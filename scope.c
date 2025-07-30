#include "scope.h"
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