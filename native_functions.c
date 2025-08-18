#include "native_functions.h"
#include "lang_types.h"
#include "utils.h"
#include "vm.h"
#include <time.h>

#define UNUSED(x) ((void)(x))

value_t native_clock(int argc, value_t* argv){
    UNUSED(argv);
    if(argc != 0)
        interpret_error_printf(get_vm_codeline(), "Expected 0 arguments in 'clock' function call, found %d\n",argc);
    return VALUE_NUMBER(clock() / CLOCKS_PER_SEC);
}

value_t native_print(int argc, value_t* argv){
    for(;--argc >= 0;){
        value_t val = argv[argc];
        if(IS_NUMBER(val)){
            printf("%d", AS_NUMBER(val));
        }else if(IS_BOOLEAN(val)){
            printf("%s", AS_BOOLEAN(val) ? "true" : "false");
        }else if(IS_OBJ(val)){
            switch (AS_OBJ(val)->type) {
                case OBJ_STRING: printf("%s", AS_OBJSTRING(val)->str); break;
                case OBJ_INSTANCE: printf("Instance of class %s", AS_OBJINSTANCE(val)->impl->name->str); break;
                case OBJ_CLASS: printf("Class %s", AS_OBJCLASS(val)->name->str); break;
                default: fatal_printf("Undefined obj_type in print()!\n");
            }
        }else if (IS_NULL(val)){
            interpret_error_printf(get_vm_codeline(), "Cannot print this expression!\n");
        }else{
            printf("print: Not implemented instruction :(");
        }
    }
    return VALUE_NULL;
}

value_t native_println(int argc, value_t* argv){
    native_print(argc,argv);
    putchar('\n');
    return VALUE_NULL;
}