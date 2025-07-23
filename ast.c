#include "ast.h"
#include "bytecode.h"
#include "token.h"
#include "utils.h"
#include "scanner.h"
#include <stdio.h>
#include <stdlib.h>

extern struct token cur_token;

#define UNDEFINED_AST_NODE_TYPE() fatal_printf("Undefined ast_node_type!\n")

ast_node* ast_mknode(ast_node_type type, ast_data data){
    ast_node* p = malloc(sizeof(ast_node));
    if(p == NULL)
        MALLOC_ERROR();
    p->data = data;
    p->type = type;
    return p;
}
void ast_freenode(ast_node* node){
    switch (node->type) {
        case AST_NUMBER: case AST_BOOLEAN:
            break;
        case AST_ADD: case AST_SUB: case AST_MUL: case AST_DIV:
            ast_freenode(((struct ast_binary*)node->data.ptr)->left);
            ast_freenode(((struct ast_binary*)node->data.ptr)->right);
            free(node->data.ptr);
            break;
        case AST_PRINT:
            ast_freenode(node->data.ptr);
            break;
        default:
            UNDEFINED_AST_NODE_TYPE();
            break;
    }
    free(node);
}

ast_node* ast_mknode_number(int val){
    ast_node* res = ast_mknode(AST_NUMBER, (ast_data){.val = VALUE_NUMBER(val)});
    return res;
}

ast_node* ast_mknode_boolean(bool val){
    ast_node* res = ast_mknode(AST_BOOLEAN, (ast_data){.val = VALUE_BOOLEAN(val)});
    return res;
}

ast_node* ast_mknode_binary(ast_node_type bin_op, ast_node* left, ast_node* right){
#ifdef DEBUG
    if(!(AST_IS_BIN_OP(bin_op)))
        fatal_printf("Incorrect binary operation in ast_mknode_binary()!\n");
    if(left == NULL || right == NULL)
        fatal_printf("One of ast_node's == NULL in ast_mknode_binary()!\n");
#endif
    ast_data binop_data;
    binop_data.ptr = malloc(sizeof(struct ast_binary));
    if(binop_data.ptr == NULL)
        MALLOC_ERROR();
    ast_node* res = ast_mknode(bin_op, binop_data);

    ((struct ast_binary*)res->data.ptr)->left = left;
    ((struct ast_binary*)res->data.ptr)->right = right;
    return res;
}

ast_node* ast_mknode_print(ast_node* expr){
    ast_node* node = ast_mknode(AST_PRINT, (ast_data){.ptr = expr});
    if(!is_match(T_SEMI))
        compile_error_printf("';' expected\n");
    return node;
}

void ast_debug_tree(const ast_node* node){
    #define DEBUG_BINARY(op) do{ \
        putchar('(');\
        ast_debug_tree(((struct ast_binary*)node->data.ptr)->left);\
        printf(" " #op " ");\
        ast_debug_tree(((struct ast_binary*)node->data.ptr)->right);\
        putchar(')');\
    }while(0)

    switch (node->type) {
        case AST_NUMBER: printf("%d", AS_NUMBER(node->data.val)); break;
        case AST_BOOLEAN: printf("%s", AS_BOOLEAN(node->data.val) ? "true" : "false"); break;
        case AST_ADD: DEBUG_BINARY(+); break;
        case AST_SUB: DEBUG_BINARY(-); break;
        case AST_MUL: DEBUG_BINARY(*); break;
        case AST_DIV: DEBUG_BINARY(/); break;
        case AST_PRINT: printf("print "); ast_debug_tree(node->data.ptr); printf(";\n"); break;
        default:
            UNDEFINED_AST_NODE_TYPE();
    }
}
