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
    T_EQUAL = 8,
    T_NEQUAL = 9,
    T_GREATER = 10,
    T_EGREATER = 11,
    T_LESS = 12, 
    T_ELESS = 13,
    T_ASSIGN = 14,
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
    T_LBRACE,
    T_RBRACE,
    T_EOF,
} token_type; 

#define TOKEN_IS_BIN_OP(op) (T_ADD <= (op) && (op) <= T_ASSIGN)

struct token{
    token_type type;
    union{
        int num;
        void* ptr;
    }data;
};

#endif