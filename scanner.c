#include "scanner.h"
#include "token.h"
#include "utils.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

struct token cur_token;
int line_counter = 1;

#define INPUT_BUF_SZ 16

static FILE* _scan_fp = NULL;

struct _input_buf{
    int sz;
    int buf[INPUT_BUF_SZ];
}_input_buf;

static inline int _get();
static inline int _skip();
static inline void _putback(int c);
static inline int _readint(int c); // last character in the stream must be a digit


static inline int _get(){
    int c = _input_buf.sz > 0 ? _input_buf.buf[--_input_buf.sz] : getc(_scan_fp);
    if(c == '\n')
        ++line_counter ;
    return c;
}

static inline void _putback(int c){
    if(_input_buf.sz >= INPUT_BUF_SZ)
        fatal_printf("input buffer size limit exceeded!\n");
    _input_buf.buf[_input_buf.sz++] = c;
}

static inline int _skip(){
    int c;
    while ((c = _get()) && isspace(c));
    return c;
}

static inline int _readint(int c){
    int val = 0;

    for(; isdigit(c); c = _get())
        val = val * 10 + (c - '0');

    _putback(c);
    return val;
}

void scanner_init(FILE* fp){
    _scan_fp = fp;
    line_counter = 1;
}

int scanner_next_token(struct token* t){
    int c = _skip();

    switch (c) {
        case '+': t->type = T_ADD; break;
        case '-': t->type = T_SUB; break;
        case '/': t->type = T_DIV; break;
        case '*': t->type = T_MUL; break;
        case '(': t->type = T_LPAR; break;
        case ')': t->type = T_RPAR; break;
        case EOF: t->type = T_EOF; return 0;
        default: 
            if(isdigit(c)){
                t->type = T_INT;
                t->data = _readint(c);
            }else{
                compile_error_printf("undefined character: %c\n", c);
            }
            break;
    }
    return 1;
}

void scanner_debug_tokens(){
    printf("Debug tokens:\n");
    while (scanner_next_token(&cur_token)) {
        switch (cur_token.type) {
            case T_INT: printf("'%d' ", cur_token.data); break;
            case T_ADD: printf("'+' "); break;
            case T_SUB: printf("'-' "); break;
            case T_MUL: printf("'*' "); break;
            case T_DIV: printf("'/' "); break;
            case T_LPAR: printf("'(' "); break;
            case T_RPAR: printf("')' "); break;
            default:
                fatal_printf("Undefined token in scanner_debug_tokens()!\n");
        }
    }
    putchar('\n');
}