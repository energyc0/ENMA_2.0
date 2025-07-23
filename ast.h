#ifndef AST_H
#define AST_H

#include "bytecode.h"

typedef enum {
    /*ast node contains pointer to a ast_binary struct*/
    AST_ADD = 0, 
    AST_SUB = 1,    
    AST_MUL = 2,
    AST_DIV = 3,

    AST_AND = 4,
    AST_OR = 5,
    AST_XOR = 6,
    AST_NOT = 7,
    /*values */
    AST_NUMBER,
    AST_BOOLEAN,
    AST_STRING,
    /*statements*/
    AST_PRINT //ast node contains pointer to an expression node
}ast_node_type;

#define AST_IS_BIN_OP(op) (AST_ADD <= (op) && (op) <= AST_XOR)

typedef union{
    void* ptr;
    value_t val;
}ast_data;


typedef struct ast_node{
    ast_node_type type;
    ast_data data;
} ast_node;

struct ast_binary{
    //operands
    ast_node* left; 
    ast_node* right;
};

ast_node* ast_mknode(ast_node_type type, ast_data data);
void ast_freenode(ast_node* node);

ast_node* ast_mknode_number(int);
ast_node* ast_mknode_boolean(bool val);
ast_node* ast_mknode_binary(ast_node_type bin_op, ast_node* left, ast_node* right);
ast_node* ast_mknode_print(ast_node* expr);

//generate ast, must be called after scanner_init()
//ast_node* generate_ast();

/*
generates ast_node of binary type
must 
*/
void ast_debug_tree(const ast_node* node);

#endif