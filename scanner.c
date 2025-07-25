#include "scanner.h"
#include "hash_table.h"
#include "lang_types.h"
#include "token.h"
#include "utils.h"
#include "symtable.h"
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct token cur_token;
int line_counter = 1;

#define INPUT_BUF_SZ (16)
#define WORD_SIZE (1024)
#define TAB_SIZE (4)
static FILE* _scan_fp = NULL;

struct _input_buf{
    int sz;
    int buf[INPUT_BUF_SZ];
}_input_buf;

static inline int _get(); //return character in the stream
static inline int _skip(); //skip spaces and return the last character
static inline int _skip_until(int c); //skip all the characters until 'c' character or EOF, return 'c' or EOF
static inline void _putback(int c); //puts character back in the stream
static inline int _readint(int c); // last character in the stream must be a digit
static inline char* _readword(int c); //first character must be alphabetical
//reads until '\"' or EOF and return a string without null byte, last character is put in the stream
//writes size of the string in the sz
static inline char* _readstring(size_t* sz);

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

static inline int _skip_until(int target){
    int c;
    for(c = _get(); c != EOF && c != target; c = _get());
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
static inline char* _readstring(size_t* sz){
    #define ADD_CHAR(c) (temp_buf[(*sz)++] = c)
    static size_t capacity = 0;
    static char* temp_buf = NULL;
    *sz = 0;

    int c;
    for(c = _get(); c != EOF && c != '\"'; c =_get()){
        if(capacity <= *sz){
            capacity += WORD_SIZE;
            temp_buf = erealloc(temp_buf, capacity);
        }
        if (c == '\\') {
            c = _get();
            switch (c) {
                case '\\':  c = '\\'; break;
                case 'n':   c = '\n'; break;
                case 't':   c = '\t'; break;
                case '\'':  c = '\''; break;
                case '\"':  c = '\"'; break;
                case '?':   c = '\?'; break;
                case 'a':   c = '\a'; break;
                case 'b':   c = '\b'; break;
                case 'f':   c = '\f'; break;
                case 'r':   c = '\r'; break;
                case 'v':   c = '\v'; break;
                default: compile_error_printf("Expected escape sequence\n");
            }
        }
        ADD_CHAR(c);
    }
    _putback(c);
    return temp_buf;
    #undef ADD_CHAR
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
                _skip_until('\n');
                return scanner_next_token(t);
            }else{
                _putback(c);
                t->type = T_DIV; break;
            }
        }
        case '#':{
            _skip_until('\n');
            return scanner_next_token(t);
        }
        case '*': t->type = T_MUL; break;
        case '(': t->type = T_LPAR; break;
        case ')': t->type = T_RPAR; break;
        case ';': t->type = T_SEMI; break;
        case '\"':{
            t->type = T_STRING;
            size_t sz;
            char* str = _readstring(&sz);
            if(_get() != '\"')
                compile_error_printf("Unclosed '\"'\n");
            int32_t hash = hash_string(str, sz);
            t->data.ptr = mk_objstring(str, sz,hash);
            break;
        }
        case EOF: t->type = T_EOF; return 0;
        default:{
            if(isdigit(c)){
                t->type = T_INT;
                t->data.num = _readint(c);
            }else if(isalpha(c)){
                char* word = _readword(c);
                token_type tok_type = symtable_procword(word);
                if(tok_type == T_IDENT)
                    t->data.num = symtable_addident(word);
                t->type = tok_type;
                break;
            }else{
                compile_error_printf("Undefined character: %c\n", c);
            }
            break;
        }
    }
    return 1;
}

void scanner_debug_tokens(){
    printf("Debug tokens:\n");
    while (scanner_next_token(&cur_token)) {
        switch (cur_token.type) {
            case T_INT: printf("'%d' ", cur_token.data.num); break;
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
            case T_STRING: printf("'\"%s\"' ", ((obj_string_t*)(cur_token.data.ptr))->str); break;
            case T_IDENT: printf("'%s' ", "SOME ID"); break;
            default:
                fatal_printf("Undefined token in scanner_debug_tokens()!\n");
        }
    }
    putchar('\n');
}