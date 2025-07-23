#include "scanner.h"
#include "token.h"
#include "utils.h"
#include "symtable.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

struct token cur_token;
int line_counter = 1;

#define INPUT_BUF_SZ (16)
#define WORD_SIZE (1024)
static FILE* _scan_fp = NULL;

struct _input_buf{
    int sz;
    int buf[INPUT_BUF_SZ];
}_input_buf;

static inline int _get();
static inline int _skip();
static inline void _putback(int c);
static inline int _readint(int c); // last character in the stream must be a digit
static inline char* _readword(int c); //first character must be alphabetical

static inline int _get(){
    int c = _input_buf.sz > 0 ? _input_buf.buf[--_input_buf.sz] : getc(_scan_fp);
    if(c == '\n')
        ++line_counter ;
    return c;
}

static inline void _putback(int c){
    if(_input_buf.sz >= INPUT_BUF_SZ)
        fatal_printf("input buffer size limit exceeded!\n");
    if((_input_buf.buf[_input_buf.sz++] = c) == '\n')
        --line_counter;
}

static inline int _skip(){
    int c;
    while ((c = _get()) != EOF && isspace(c));
    return c;
}

static inline int _readint(int c){
    int val = 0;

    for(; isdigit(c); c = _get())
        val = val * 10 + (c - '0');

    _putback(c);
    return val;
}

static inline char* _readword(int c){
    static char buf[WORD_SIZE];
    int sz = 0;
    for(;sz < (WORD_SIZE-1) && (isalnum(c) || c == '_'); c = _get()){
        buf[sz++] = c;
    }
    _putback(c);
    buf[sz] = '\0';
    return buf;
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
        case '/':{
            //it is a commentary 
            if((c = _get()) == '/'){
                while ((c = _get()) != EOF && c != '\n');
                return scanner_next_token(t);
            }else{
                _putback(c);
                t->type = T_DIV; break;
            }
        }
        case '*': t->type = T_MUL; break;
        case '(': t->type = T_LPAR; break;
        case ')': t->type = T_RPAR; break;
        case ';': t->type = T_SEMI; break;
        case EOF: t->type = T_EOF; return 0;
        default: 
            if(isdigit(c)){
                t->type = T_INT;
                t->data = _readint(c);
            }else if(isalpha(c)){
                char* word = _readword(c);
                token_type tok_type = symtable_procword(word);
                if(tok_type == T_IDENT)
                    t->data = symtable_addident(word);
                t->type = tok_type;
                break;
            }else{
                compile_error_printf("Undefined character: %c\n", c);
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
            case T_SEMI: printf("';' "); break;
            case T_AND: printf("'and' "); break;
            case T_OR: printf("'or' "); break;
            case T_XOR: printf("'xor' "); break;
            case T_NOT: printf("'not' "); break;
            case T_PRINT: printf("'print' "); break;
            case T_FALSE: printf("'false' "); break;
            case T_TRUE: printf("'true' "); break;
            case T_IDENT: printf("'%s' ", symtable_getident(cur_token.data)); break;
            default:
                fatal_printf("Undefined token in scanner_debug_tokens()!\n");
        }
    }
    putchar('\n');
}