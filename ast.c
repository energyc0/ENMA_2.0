#include "ast.h"
#include "bytecode.h"
#include "lang_types.h"
#include "token.h"
#include "utils.h"
#include "scanner.h"
#include <stdio.h>
#include <stdlib.h>

extern struct token cur_token;

#define UNDEFINED_AST_NODE_TYPE() fatal_printf("Undefined ast_node_type!\n")

ast_node* ast_mknode(ast_node_type type, ast_data data){
    ast_node* p = emalloc(sizeof(ast_node));
    p->data = data;
    p->type = type;
    return p;
}

void ast_freenode(ast_node* node){
    switch (node->type) {
        //garbage collector is responsible for strings
        case AST_NUMBER: case AST_BOOLEAN: case AST_STRING:
            break;
        case AST_ADD: case AST_SUB: case AST_MUL: case AST_DIV:
        case AST_AND: case AST_OR: case AST_XOR: case AST_NEQUAL:
        case AST_EQUAL: case AST_EGREATER: case AST_GREATER: case AST_ELESS:
        case AST_LESS:
            ast_freenode(((struct ast_binary*)node->data.ptr)->left);
            ast_freenode(((struct ast_binary*)node->data.ptr)->right);
            free(node->data.ptr);
            break;
        case AST_PRINT: case AST_NOT:
            ast_freenode(node->data.ptr);
            break;
        default:
            eprintf("ast_freenode() ");
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

ast_node* ast_mknode_string(obj_string_t* str){
    ast_node* res = ast_mknode(AST_STRING, (ast_data){.val = VALUE_OBJ(str)});
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
    binop_data.ptr = emalloc(sizeof(struct ast_binary));
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
        case AST_STRING: printf("\"%s\"", AS_OBJSTRING(node->data.val)->str); break;
        case AST_ADD: DEBUG_BINARY(+); break;
        case AST_SUB: DEBUG_BINARY(-); break;
        case AST_MUL: DEBUG_BINARY(*); break;
        case AST_DIV: DEBUG_BINARY(/); break;
        case AST_AND: DEBUG_BINARY(and); break;
        case AST_OR: DEBUG_BINARY(or); break;
        case AST_XOR: DEBUG_BINARY(xor); break;
        case AST_NOT: {
            printf("( not ");
            ast_debug_tree(node->data.ptr);
            printf(" )");
            break;
        }
        case AST_ASSIGN: DEBUG_BINARY(=); break;
        case AST_NEQUAL: DEBUG_BINARY(!=); break;
        case AST_EQUAL: DEBUG_BINARY(==); break;
        case AST_EGREATER: DEBUG_BINARY(>=); break;
        case AST_GREATER: DEBUG_BINARY(>); break;
        case AST_ELESS: DEBUG_BINARY(<=); break;
        case AST_LESS: DEBUG_BINARY(<); break;
        case AST_PRINT: printf("print "); ast_debug_tree(node->data.ptr); printf(";\n"); break;
        default:
            printf("ast_debug_tree(): ");
            UNDEFINED_AST_NODE_TYPE();
    }
}

