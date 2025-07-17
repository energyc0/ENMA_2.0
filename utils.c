#include "utils.h"
#include "scanner.h"
#include <stdio.h>
#include <stdarg.h>

__attribute__((noreturn)) void fatal_printf(const char* fmt, ...){
    va_list ap;
    va_start(ap, fmt);

    eprintf("FATAL: ");
    vfprintf(stderr, fmt, ap);

    va_end(ap);
    exit(1);
}
__attribute__((noreturn)) void compile_error_printf(const char* fmt, ...){
    va_list ap;
    va_start(ap, fmt);

    eprintf("Syntax error on line %d: ", line_counter);
    vfprintf(stderr, fmt, ap);

    va_end(ap);
    exit(1);
}
__attribute__((noreturn)) void user_error_printf(const char* fmt, ...){
    va_list ap;
    va_start(ap, fmt);

    vfprintf(stderr, fmt, ap);

    va_end(ap);
    exit(1);
}

__attribute__((noreturn)) void unexpected_token(){
    compile_error_printf("Unexpected token: '%s'\n", token_to_string(cur_token.type));
}