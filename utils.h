#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include "scanner.h"

extern struct token cur_token;
extern int line_counter;

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

__attribute__((noreturn)) void fatal_printf(const char* fmt, ...);
__attribute__((noreturn)) void compile_error_printf(const char* fmt, ...);
__attribute__((noreturn)) void user_error_printf(const char* fmt, ...);

//check cur_token type and print syntax error
__attribute__((noreturn)) void unexpected_token();
static inline int is_match(token_type expect){
    return cur_token.type == expect;
}

#define MALLOC_ERROR() fatal_printf("malloc() returned NULL!\n")

#endif