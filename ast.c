#include "ast.h"
#include "lang_types.h"
#include "scope.h"
#include "token.h"
#include "utils.h"
#include "symtable.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

extern struct token cur_token;

#define UNDEFINED_AST_NODE_TYPE(node) fatal_printf("Undefined ast_node_type! type number: %d\n", node->type)

static struct ast_class_info* ast_mk_classinfo(struct ast_func_arg* args, struct obj_class_t*cl);

static void ast_debug_node(const ast_node* node);
static void ast_debug_args(struct ast_func_arg* arg);

static void freeargs(struct ast_func_arg* p);

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
            freeargs(ptr->args);
            free(ptr);
            break;
        }
        case AST_CONSTRUCTOR:{
            struct ast_class_info* ptr = node->data.ptr;
            freeargs(ptr->args);
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

static struct ast_class_info* ast_mk_classinfo(struct ast_func_arg* args, struct obj_class_t*cl){
    struct ast_class_info* ptr = emalloc(sizeof(struct ast_class_info));
    ptr->args = args;
    ptr->cl = cl;
    return ptr;
}

ast_node* ast_mknode_constructor(struct ast_func_arg* args, struct obj_class_t*cl){
    return ast_mknode(AST_CONSTRUCTOR, AST_DATA_PTR(ast_mk_classinfo(args,cl)));
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
            if(ptr)
                ast_debug_args(ptr);
            putchar(')');
            break;
        }
        case AST_CONSTRUCTOR:{
            struct ast_class_info* ptr = node->data.ptr;
            printf("%s(", ptr->cl->name->str);
            if(ptr->args)
                ast_debug_args(ptr->args);
            putchar(')');
            break;
        }
        default:
            printf("ast_debug_tree(): ");
            UNDEFINED_AST_NODE_TYPE(node);
    }
}

struct ast_func_info* ast_mknode_func_info(struct obj_func_base_t* func, struct ast_func_arg* args){
    struct ast_func_info* ptr = emalloc(sizeof(struct ast_func_info));
    ptr->args = args;
    ptr->func = func;
    return ptr;
}

ast_node* ast_mknode_func(struct ast_func_arg* args, struct obj_func_base_t* func){
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
    
    #define BOOLEAN_OP(op)do{\
        value_t a; value_t b;\
        EXTRACT_OPERANDS(a, b);\
        if(!(IS_BOOLEAN(a) && IS_BOOLEAN(b))) \
            compile_error_printf("Incompatible types for operation.\n"); \
        return VALUE_BOOLEAN(AS_BOOLEAN(a) op AS_BOOLEAN(b));\
    }while(0)

    #define EXTRACT_IDENTIFIER(val)do{\
        if(!symtable_get(AS_OBJIDENTIFIER(root->data.val), &val) || IS_NULL(val))\
            compile_error_printf("Undefined identifier %s\n", AS_OBJIDENTIFIER(root->data.val)->str);\
    }while(0)

    #define EXTRACT_NUMBER_INDENTIFIER(val)do{\
        EXTRACT_IDENTIFIER(val);\
        if(!IS_NUMBER(val))\
            compile_error_printf("Incompatible types for operation.\n");\
    }while(0)

    #define POST_OP(op) do{\
        value_t val;\
        EXTRACT_NUMBER_INDENTIFIER(val);\
        value_t temp = val;\
        op AS_NUMBER(val);\
        symtable_set(AS_OBJIDENTIFIER(root->data.val), val); \
        return temp;\
    }while(0)

    #define PREF_OP(op) do{\
        value_t val;\
        EXTRACT_NUMBER_INDENTIFIER(val);\
        op AS_NUMBER(val);\
        symtable_set(AS_OBJIDENTIFIER(root->data.val), val); \
        return val;\
    }while(0)

    switch(root->type){
        case AST_NUMBER: case AST_BOOLEAN: case AST_STRING:
            return root->data.val;
        case AST_IDENT:{
            value_t val;
            EXTRACT_IDENTIFIER(val);
            return val;
        }
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
        case AST_AND:
            BOOLEAN_OP(&&);
        case AST_OR:
            BOOLEAN_OP(||);
        case AST_XOR:
            BOOLEAN_OP(^);
        case AST_NOT:{
            value_t val = ast_eval(root->data.ptr);
            if(!IS_BOOLEAN(val))
                compile_error_printf("Incompatible type for operation\n");
            return VALUE_BOOLEAN(!AS_BOOLEAN(val));
        }
        case AST_EQUAL:
            NUMERICAL_OP(==, VALUE_BOOLEAN);
        case AST_NEQUAL:
            NUMERICAL_OP(!=, VALUE_BOOLEAN);
        case AST_GREATER:
            NUMERICAL_OP(>, VALUE_BOOLEAN);
        case AST_EGREATER:
            NUMERICAL_OP(>=, VALUE_BOOLEAN);
        case AST_LESS:
            NUMERICAL_OP(<, VALUE_BOOLEAN);
        case AST_ELESS:
            NUMERICAL_OP(<=, VALUE_BOOLEAN);
        case AST_ASSIGN:{
            value_t a; value_t b;
            EXTRACT_OPERANDS(a,b);
            if(!IS_OBJIDENTIFIER(a))
                compile_error_printf("Left value is not assignable\n");
            value_t val;
            if(!symtable_get(AS_OBJIDENTIFIER(a), &val) || IS_NULL(val))
                compile_error_printf("Undefined identifier '%s'\n", AS_OBJIDENTIFIER(a)->str);
            if(!is_value_same_type(b,val))
                compile_error_printf("Incorrect type for assignment\n");
            symtable_set(AS_OBJIDENTIFIER(a), b);
            return b;
        }
        case AST_POSTINCR:
            POST_OP(++);
        case AST_PREFINCR:
            PREF_OP(++);
        case AST_POSTDECR:
            POST_OP(--);
        case AST_PREFDECR:
            PREF_OP(--);
        default:
            fatal_printf("Undefined ast_type in ast_eval()!\nast_type = %d\n", root->type);
    }
}

static void ast_debug_args(struct ast_func_arg* arg){
    if(arg->next){
        ast_debug_args(arg->next);
        printf(", ");
    }
    ast_debug_node(arg->arg);
}

static void freeargs(struct ast_func_arg* p){
    while(p){
        struct ast_func_arg* temp = p->next;
        ast_freenode(p->arg);
        free(p);
        p = temp;
    }
}