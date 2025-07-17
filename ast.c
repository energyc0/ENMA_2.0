#include "ast.h"
#include "bytecode.h"
#include "token.h"
#include "utils.h"
#include "scanner.h"
#include <stdio.h>
#include <stdlib.h>

extern struct token cur_token;

#define MALLOC_ERROR() fatal_printf("malloc() returned NULL!\n")
#define UNDEFINED_AST_NODE_TYPE() fatal_printf("Undefined ast_node_type!\n")

ast_node* ast_mknode(ast_node_type type, void* data){
    ast_node* p = malloc(sizeof(ast_node));
    if(p == NULL)
        MALLOC_ERROR();
    p->data = data;
    p->type = type;
    return p;
}
void ast_freenode(ast_node* node){
    switch (node->type) {
        case AST_CONSTANT:
            free(node->data);
            break;
        case AST_ADD: case AST_SUB: case AST_MUL: case AST_DIV:
            ast_freenode(((struct ast_binary*)node->data)->left);
            ast_freenode(((struct ast_binary*)node->data)->right);
            free(node->data);
            break;
        case AST_PRINT:
            ast_freenode(node->data);
            break;
        default:
            UNDEFINED_AST_NODE_TYPE();
            break;
    }
    free(node);
}

ast_node* ast_mknode_constant(vm_word_t constant){
    vm_word_t* temp = malloc(sizeof(vm_word_t));
    if(temp == NULL)
        MALLOC_ERROR();
    *temp = constant;

    ast_node* res = ast_mknode(AST_CONSTANT, temp);
    return res;
}

ast_node* ast_mknode_binary(ast_node_type bin_op, ast_node* left, ast_node* right){
#ifdef DEBUG
    if(!(AST_IS_BIN_OP(bin_op)))
        fatal_printf("Incorrect binary operation in ast_mknode_binary()!\n");
    if(left == NULL || right == NULL)
        fatal_printf("One of ast_node's == NULL in ast_mknode_binary()!\n");
#endif
    ast_node* res = malloc(sizeof(ast_node));
    if(res == NULL)
        MALLOC_ERROR();

    res->type = bin_op;
    res->data = malloc(sizeof(struct ast_binary));
    if(res->data == NULL)
        MALLOC_ERROR();

    ((struct ast_binary*)res->data)->left = left;
    ((struct ast_binary*)res->data)->right = right;
    return res;
}

ast_node* ast_mknode_print(ast_node* expr){
    ast_node* node = ast_mknode(AST_PRINT, expr);
    if(!is_match(T_SEMI))
        compile_error_printf("';' expected\n");
    return node;
}

void ast_debug_tree(const ast_node* node){
    #define DEBUG_BINARY(op) do{ \
        putchar('(');\
        ast_debug_tree(((struct ast_binary*)node->data)->left);\
        printf(" " #op " ");\
        ast_debug_tree(((struct ast_binary*)node->data)->right);\
        putchar(')');\
    }while(0)

    switch (node->type) {
        case AST_CONSTANT: printf("%d", *(vm_word_t*)node->data); break;
        case AST_ADD: DEBUG_BINARY(+); break;
        case AST_SUB: DEBUG_BINARY(-); break;
        case AST_MUL: DEBUG_BINARY(*); break;
        case AST_DIV: DEBUG_BINARY(/); break;
        case AST_PRINT: printf("print "); ast_debug_tree(node->data); printf(";\n"); break;
        default:
            UNDEFINED_AST_NODE_TYPE();
    }
}
