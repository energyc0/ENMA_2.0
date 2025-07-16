#include "utils.h"
#include "scanner.h"
#include <stdio.h>
#include <stdarg.h>

extern int line_counter;

void fatal_printf(const char* fmt, ...){
    va_list ap;
    va_start(ap, fmt);

    eprintf("FATAL: ");
    vfprintf(stderr, fmt, ap);

    va_end(ap);
    exit(1);
}
void compile_error_printf(const char* fmt, ...){
    va_list ap;
    va_start(ap, fmt);

    eprintf("Syntax error on line %d: ", line_counter);
    vfprintf(stderr, fmt, ap);

    va_end(ap);
    exit(1);
}
void user_error_printf(const char* fmt, ...){
    va_list ap;
    va_start(ap, fmt);

    vfprintf(stderr, fmt, ap);

    va_end(ap);
    exit(1);
}