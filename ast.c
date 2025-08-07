#include "ast.h"
#include "bytecode.h"
#include "lang_types.h"
#include "scope.h"
#include "token.h"
#include "utils.h"
#include "scanner.h"
#include <stdio.h>
#include <stdlib.h>

extern struct token cur_token;

#define UNDEFINED_AST_NODE_TYPE(node) fatal_printf("Undefined ast_node_type! type number: %d\n", node->type)

static void ast_debug_node(const ast_node* node);

ast_node* ast_mknode(ast_node_type type, ast_data data){
    ast_node* p = emalloc(sizeof(ast_node));
    p->data = data;
    p->type = type;
    return p;
}

void ast_freenode(ast_node* node){
    switch (node->type) {
        //garbage collector is responsible for these
        case AST_NUMBER: case AST_BOOLEAN: case AST_STRING: case AST_IDENT:
        case AST_POSTDECR: case AST_POSTINCR: case AST_PREFINCR: case AST_PREFDECR:
            break;
        case AST_ADD: case AST_SUB: case AST_MUL: case AST_DIV:
        case AST_AND: case AST_OR: case AST_XOR: case AST_NEQUAL:
        case AST_EQUAL: case AST_EGREATER: case AST_GREATER: case AST_ELESS:
        case AST_LESS: case AST_ASSIGN:
            ast_freenode(((struct ast_binary*)node->data.ptr)->left);
            ast_freenode(((struct ast_binary*)node->data.ptr)->right);
            free(node->data.ptr);
            break;
        case AST_NOT:
            ast_freenode(node->data.ptr);
            break;
        case AST_CALL:{
            struct ast_func_info* ptr = node->data.ptr;
            struct ast_func_arg* p = ptr->args;
            while(p){
                struct ast_func_arg* temp = p->next;
                ast_freenode(p->arg);
                free(p);
                p = temp;
            }
            free(ptr);
            break;
        }
        default:
            eprintf("ast_freenode() ");
            UNDEFINED_AST_NODE_TYPE(node);
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

ast_node* ast_mknode_identifier(obj_string_t* id){
    ast_node* node = ast_mknode(AST_IDENT, AST_DATA_VALUE(VALUE_OBJ(id)));
    return node;
}

void ast_debug_tree(const ast_node* node){
    printf("Parsed expression at line %d with depth %d:\n", line_counter, get_scope());
    ast_debug_node(node);
    putchar('\n');
}

static void ast_debug_node(const ast_node* node){
    #define DEBUG_BINARY(op) do{ \
        putchar('(');\
        ast_debug_node(((struct ast_binary*)node->data.ptr)->left);\
        printf(" " #op " ");\
        ast_debug_node(((struct ast_binary*)node->data.ptr)->right);\
        putchar(')');\
    }while(0)

    switch (node->type) {
        case AST_NUMBER: printf("%d", AS_NUMBER(node->data.val)); break;
        case AST_BOOLEAN: printf("%s", AS_BOOLEAN(node->data.val) ? "true" : "false"); break;
        case AST_STRING: printf("\"%s\"", AS_OBJSTRING(node->data.val)->str); break;
        case AST_IDENT: printf("%s", AS_OBJIDENTIFIER(node->data.val)->str); break; 
        case AST_ADD: DEBUG_BINARY(+); break;
        case AST_SUB: DEBUG_BINARY(-); break;
        case AST_MUL: DEBUG_BINARY(*); break;
        case AST_DIV: DEBUG_BINARY(/); break;
        case AST_AND: DEBUG_BINARY(and); break;
        case AST_OR: DEBUG_BINARY(or); break;
        case AST_XOR: DEBUG_BINARY(xor); break;
        case AST_NOT: {
            printf("( not ");
            ast_debug_node(node->data.ptr);
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
        case AST_PREFDECR:  printf("--%s", AS_OBJIDENTIFIER(node->data.val)->str); break;
        case AST_PREFINCR:  printf("++%s", AS_OBJIDENTIFIER(node->data.val)->str); break;
        case AST_POSTDECR:  printf("%s--", AS_OBJIDENTIFIER(node->data.val)->str); break;
        case AST_POSTINCR:  printf("%s++", AS_OBJIDENTIFIER(node->data.val)->str); break;
        case AST_CALL:{
            printf("%s(", ((struct ast_func_info*)node->data.ptr)->func->name->str);
            struct ast_func_arg* ptr = ((struct ast_func_info*)node->data.ptr)->args;
            while(ptr){
                ast_debug_node(ptr->arg);
                if(ptr->next)
                    printf(", ");
                ptr = ptr->next;
            }
            putchar(')');
            break;
        }
        default:
            printf("ast_debug_tree(): ");
            UNDEFINED_AST_NODE_TYPE(node);
    }
}

struct ast_func_info* ast_mknode_func_info(obj_function_t* func, struct ast_func_arg* args){
    struct ast_func_info* ptr = emalloc(sizeof(struct ast_func_info));
    ptr->args = args;
    ptr->func = func;
    return ptr;
}

ast_node* ast_mknode_func(struct ast_func_arg* args, obj_function_t* func){
    return ast_mknode(AST_CALL, AST_DATA_PTR(ast_mknode_func_info(func, args)));
}

struct ast_func_arg* ast_mknode_func_arg(ast_node* node){
    struct ast_func_arg* ptr = emalloc(sizeof(struct ast_func_arg));
    ptr->next = NULL;
    ptr->arg = node;
    return ptr;
}

//TODO ALL OPERATIONS
value_t ast_eval(ast_node* root){
    #define EXTRACT_OPERANDS(a,b)do{\
        struct ast_binary* p = root->data.ptr;\
        a = ast_eval(p->left); b = ast_eval(p->right); \
    }while(0)

    #define NUMERICAL_OP(op, ret_type)do{ \
        value_t a; value_t b;\
        EXTRACT_OPERANDS(a, b);\
        if(!(IS_NUMBER(a) && IS_NUMBER(b))) \
            compile_error_printf("Incompatible types for operation.\n"); \
        return ret_type (AS_NUMBER(a) op AS_NUMBER(b));\
    }while(0)

    switch(root->type){
        case AST_NUMBER: case AST_BOOLEAN: case AST_STRING:
            return root->data.val;
        case AST_SUB:
            NUMERICAL_OP(-, VALUE_NUMBER);
        case AST_DIV:
            NUMERICAL_OP(/, VALUE_NUMBER);
        case AST_MUL:
            NUMERICAL_OP(*, VALUE_NUMBER);
        case AST_ADD:{
                value_t a; value_t b;
                EXTRACT_OPERANDS(a, b);
                if(IS_NUMBER(a) && IS_NUMBER(b)){
                    return VALUE_NUMBER(AS_NUMBER(a) + AS_NUMBER(b));
                }else if(IS_OBJSTRING(a) && IS_OBJSTRING(b)){
                    return VALUE_OBJ(objstring_conc(a,b));
                }else{
                    compile_error_printf("Incompatible types for operation.\n");\
                }
        }
        default:
            fatal_printf("Undefined ast_type in ast_eval()!\nast_type = %d\n", root->type);
    }
}