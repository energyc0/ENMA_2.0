#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include "scanner.h"
#include "token.h"

extern struct token cur_token;
extern int line_counter;

#define ARR_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

__attribute__((noreturn)) void fatal_printf(const char* fmt, ...);
__attribute__((noreturn)) void compile_error_printf(const char* fmt, ...);
__attribute__((noreturn)) void interpret_error_printf(int line, const char* fmt, ...);
__attribute__((noreturn)) void user_error_printf(const char* fmt, ...);

static inline int is_match(token_type expect){
    return cur_token.type == expect;
}

static inline void cur_expect(token_type expect, const char* err_msg){
    if(!is_match(expect))
        compile_error_printf(err_msg);
}

static inline void next_expect(token_type expect, const char* err_msg){
    if(!scanner_next_token() || !is_match(expect))
        compile_error_printf(err_msg);
}

void* emalloc(size_t count);
void* erealloc(void* ptr, size_t count);

#define PRRED "\x1b[0;31m"
#define PRGREEN "\x1b[0;32m"
#define PRBLUE "\x1b[0;34m"

void color_printf(const char* color, const char* fmt, ...);

#define red_printf(...) color_printf(PRRED, __VA_ARGS__)
#define green_printf(...) color_printf(PRGREEN, __VA_ARGS__)
#define blue_printf(...) color_printf(PRBLUE, __VA_ARGS__)

#ifdef DEBUG
    #define dprintf(...) printf(__VA_ARGS__)
#else
    #define dprintf(...) do {} while (0)
#endif
#endif