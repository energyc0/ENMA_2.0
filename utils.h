#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include "scanner.h"

extern struct token cur_token;
extern int line_counter;

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

void fatal_printf(const char* fmt, ...);
void compile_error_printf(const char* fmt, ...);
void user_error_printf(const char* fmt, ...);

//check cur_token type and print syntax error
void unexpected_token();
static inline int is_match(token_type expect){
    return cur_token.type == expect;
}

#endif