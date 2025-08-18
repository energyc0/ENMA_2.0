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
    T_DOT = 15,
    T_INCR,
    T_DECR,
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
    T_VAR,
    T_IF,
    T_ELSE,
    T_WHILE,
    T_FOR,
    T_BREAK,
    T_CONTINUE,
    T_FUNC,
    T_RETURN,
    T_CLASS,
    T_FIELD,
    
    //other
    T_SEMI,
    T_COMMA,
    T_LBRACE, // '{'
    T_RBRACE, // '}'
    T_EOF,
} token_type; 

#define TOKEN_IS_BIN_OP(op) (T_ADD <= (op) && (op) <= T_DOT)

struct token{
    token_type type;
    union{
        int num;
        void* ptr;
    }data;
};

#endif