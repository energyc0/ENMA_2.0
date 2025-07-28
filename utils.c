#include "utils.h"
#include "scanner.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

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

//__attribute__((noreturn)) void unexpected_token(){
//    compile_error_printf("Unexpected token: '%s'\n", token_to_string(cur_token.type));
//}

void* emalloc(size_t count){
    void* ptr = malloc(count);
    if(ptr == NULL)
        fatal_printf("malloc() returned NULL!\n");
    return ptr;
}
void* erealloc(void* ptr, size_t count){
    void* temp = realloc(ptr, count);
    if(temp == NULL){
        free(ptr);
        fatal_printf("realloc() returned NULL!\n");
    }
    return temp;
}