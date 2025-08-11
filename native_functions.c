#include "native_functions.h"
#include "utils.h"
#include "vm.h"
#include <time.h>

value_t native_clock(int argc, value_t* argv){
    if(argc != 0)
        interpret_error_printf(get_vm_codeline(), "Expected 0 arguments in 'clock' function call, found %d\n",argc);
    return VALUE_NUMBER(clock() / CLOCKS_PER_SEC);
}