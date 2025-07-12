#ifndef TOKEN_H
#define TOKEN_H

typedef enum token_t{
    T_NULL,
    T_INT,
    T_ADD,
    T_SUB,
    T_MUL,
    T_DIV,
    T_LPAR,
    T_RPAR
} token_t; 

struct token{
    token_t type;
    int data;
};

#endif