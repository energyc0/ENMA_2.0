#ifndef TOKEN_H
#define TOKEN_H

typedef enum{
    //binary operations
    T_ADD = 0,
    T_SUB = 1,
    T_MUL = 2,
    T_DIV = 3,
    //logical operators
    T_AND = 4,
    T_OR = 5,
    T_XOR = 6,
    T_NOT = 7,
    //precedence operators
    T_LPAR,
    T_RPAR,
    //values
    T_INT,
    T_FALSE,
    T_TRUE,
    T_IDENT,
    T_STRING,
    //keywords
    T_PRINT,
    //other
    T_SEMI,
    T_EOF,
} token_type; 

#define TOKEN_IS_BIN_OP(op) (T_ADD <= (op) && (op) <= T_XOR)

struct token{
    token_type type;
    union{
        int num;
        void* ptr;
    }data;
};

const char* token_to_string(token_type type);

#endif