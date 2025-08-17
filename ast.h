#ifndef AST_H
#define AST_H

#include "bytecode.h"
#include "lang_types.h"

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
    AST_EQUAL = 8,
    AST_NEQUAL = 9,
    AST_GREATER = 10,
    AST_EGREATER = 11,
    AST_LESS = 12, 
    AST_ELESS = 13,
    AST_ASSIGN = 14,

    /*ast node contains pointer to obj_id_t*/
    AST_POSTINCR,
    AST_PREFINCR,
    AST_POSTDECR,
    AST_PREFDECR,
    AST_PROPERTY,
    /*values */
    AST_NUMBER,
    AST_BOOLEAN,
    AST_STRING,
    AST_IDENT,

    AST_CALL,
    AST_CONSTRUCTOR,
}ast_node_type;

#define AST_IS_BIN_OP(op) (AST_ADD <= (op) && (op) <= AST_PROPERTY)

typedef union{
    void* ptr;
    value_t val;
}ast_data;

#define AST_DATA_VALUE(value) ((ast_data){.val = (value)})
#define AST_DATA_PTR(value) ((ast_data){.ptr = (value)})

typedef struct ast_node{
    ast_node_type type;
    ast_data data;
} ast_node;

struct ast_binary{
    //operands
    ast_node* left; 
    ast_node* right;
};

struct ast_func_arg{
    ast_node* arg;
    struct ast_func_arg* next;
};

struct ast_func_info{
    struct obj_func_base_t* func;
    struct ast_func_arg* args;
};

struct ast_class_info{
    struct obj_class_t* cl;
    struct ast_func_arg* args;
};

struct ast_property{
    obj_id_t* instance;
    struct ast_property* property;
};

ast_node* ast_mknode(ast_node_type type, ast_data data);
void ast_freenode(ast_node* node);

ast_node* ast_mknode_number(int);
ast_node* ast_mknode_boolean(bool val);
ast_node* ast_mknode_string(obj_string_t* str);
ast_node* ast_mknode_identifier(obj_string_t* id);
ast_node* ast_mknode_binary(ast_node_type bin_op, ast_node* left, ast_node* right);
ast_node* ast_mknode_func(struct ast_func_arg*, struct obj_func_base_t*);
ast_node* ast_mknode_constructor(struct ast_func_arg*, struct obj_class_t*);
ast_node* ast_mknode_property(struct ast_property*);

struct ast_func_arg* ast_mknode_func_arg(ast_node* node);
struct ast_func_info* ast_mknode_func_info(struct obj_func_base_t*, struct ast_func_arg*);
struct ast_property* ast_mk_property(obj_id_t* instance);

value_t ast_eval(ast_node* root);

void ast_debug_tree(const ast_node* node);

#endif