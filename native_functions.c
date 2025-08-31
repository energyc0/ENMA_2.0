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
#ifdef DEBUG
    #define natprint(...) red_printf(__VA_ARGS__)
#else
    #define natprint(...) printf(__VA_ARGS__)
#endif

    for(;--argc >= 0;){
        value_t val = argv[argc];
        if(IS_NUMBER(val)){
            natprint("%d", AS_NUMBER(val));
        }else if(IS_BOOLEAN(val)){
            natprint("%s", AS_BOOLEAN(val) ? "true" : "false");
        }else if(IS_OBJ(val)){
            switch (AS_OBJ(val)->type) {
                case OBJ_STRING: natprint("%s", AS_OBJSTRING(val)->str); break;
                case OBJ_INSTANCE: natprint("Instance of class %s", AS_OBJINSTANCE(val)->impl->name->str); break;
                case OBJ_CLASS: natprint("Class %s", AS_OBJCLASS(val)->name->str); break;
                default: fatal_printf("Undefined obj_type in print()!\n");
            }
        }
        else if(IS_UNINIT(val)){
            natprint("NULL");
        }else if (IS_NONE(val)){
            interpret_error_printf(get_vm_codeline(), "Cannot print this expression!\n");
        }else{
            natprint("print: Not implemented :(");
        }
    }
    return VALUE_NONE;

#undef natprint
}

value_t native_println(int argc, value_t* argv){
    native_print(argc,argv);
    putchar('\n');
    return VALUE_NONE;
}


value_t native_isnum(int argc, value_t* argv){
    if(argc != 1)
        interpret_error_printf(get_vm_codeline(),
     "Expected 1 argument in 'isnum' function call, found %d\n", argc);
    return VALUE_BOOLEAN(IS_NUMBER(argv[0]));
}
value_t native_isstr(int argc, value_t* argv){
    if(argc != 1)
        interpret_error_printf(get_vm_codeline(),
    "Expected 1 argument in 'isstr' function call, found %d\n", argc);
    return VALUE_BOOLEAN(IS_OBJSTRING(argv[0]));
}
value_t native_isbool(int argc, value_t* argv){
    if(argc != 1)
        interpret_error_printf(get_vm_codeline(),
     "Expected 1 argument in 'isbool' function call, found %d\n", argc);
    return VALUE_BOOLEAN(IS_BOOLEAN(argv[0]));
}
value_t native_isinst(int argc, value_t* argv){
    if(argc != 1)
        interpret_error_printf(get_vm_codeline(),
     "Expected 1 argument in 'isinst' function call, found %d\n", argc);
    return VALUE_BOOLEAN(IS_OBJINSTANCE(argv[0]));
}