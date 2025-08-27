#include "utils.h"
#include "garbage_collector.h"
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

    eprintf("Syntax error at line %d: ", line_counter);
    vfprintf(stderr, fmt, ap);

    va_end(ap);
    exit(1);
}

__attribute__((noreturn)) void interpret_error_printf(int line, const char* fmt, ...){
    va_list ap;
    va_start(ap, fmt);

    eprintf("Error at line %d: ", line);
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

void color_printf(const char* color, const char* fmt, ...){
    va_list ap;
    va_start(ap, fmt);

    printf("%s",color);
    vfprintf(stdout, fmt, ap);
    printf("\x1b[0m");

    va_end(ap);
}