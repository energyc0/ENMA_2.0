#ifndef TOKEN_H
#define TOKEN_H

typedef enum{
    T_EOF,
    //binary operations
    T_ADD,
    T_SUB,
    T_MUL,
    T_DIV,
    //precedence operators
    T_LPAR,
    T_RPAR,
    //values
    T_INT,
    T_FALSE,
    T_TRUE,
    T_IDENT,
    //keywords
    T_PRINT,
    //other
    T_SEMI
    
} token_type; 

#define TOKEN_IS_BIN_OP(op) (T_ADD <= (op) && (op) <= T_DIV)

struct token{
    token_type type;
    int data;
};

const char* token_to_string(token_type type);

#endif