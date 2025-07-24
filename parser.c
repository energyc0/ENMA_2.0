#include "parser.h"
#include "ast.h"
#include "bytecode.h"
#include "scanner.h"
#include "symtable.h"
#include "token.h"
#include "utils.h"

static const int precedence[] = {
    5,  //T_ADD
    5,  //T_SUB
    6,  //T_MUL
    6,  //T_DIV
    3,  //T_AND
    2,  //T_OR
    1,  //T_XOR
    4,  //T_NOT
    0,  //T_LPAR
    -1, //T_RPAR
    0,  //T_T_INT
    0,  //T_FALSE
    0,  //T_TRUE
    0,  //T_IDENT
    0,  //T_STRING
    0,  //T_PRINT
    -1,  //T_SEMI
    0  //T_EOF
};
extern struct token cur_token;

static inline int get_op_precedence(token_type op);

static ast_node* ast_bin_expr(int prev_precedence);
static ast_node* ast_primary();

static ast_node* ast_bin_expr(int prev_precedence){
    ast_node* right;
    ast_node* left = ast_primary();

    if(!scanner_next_token(&cur_token))
        return left;
    
    token_type cur_op = cur_token.type;
    
    while(get_op_precedence(cur_op) > prev_precedence){
        right = ast_bin_expr(precedence[cur_op]);

        left = ast_mknode_binary((ast_node_type)cur_op, left, right);

        cur_op = cur_token.type;
        if(!TOKEN_IS_BIN_OP(cur_op))
            break;
    }
    return left;
}

static ast_node* ast_primary(){
    if(!scanner_next_token(&cur_token))
        compile_error_printf("Expected expression\n");

    ast_node* temp;
    switch (cur_token.type) {
        case T_INT: 
            return ast_mknode_number(cur_token.data);
        case T_FALSE:
            return ast_mknode_boolean(false);
        case T_TRUE:
            return ast_mknode_boolean(true);
        case T_LPAR: 
            temp = ast_bin_expr(0);
            break;
        case T_NOT:
            return ast_mknode(AST_NOT, (ast_data){.ptr = ast_primary()});
        case T_STRING:
            return ast_mknode_string(symtable_getstring(cur_token.data));
        case T_SUB:
            temp = ast_primary();
            if(temp->type == AST_NUMBER){
                AS_NUMBER(temp->data.val) *= -1; 
                return temp;
            }else{
                return ast_mknode_binary(AST_MUL, ast_mknode_number(-1), temp);
            }
        default:
            compile_error_printf("Expected expression\n");
    }
    if(!is_match(T_RPAR))
        compile_error_printf("Unclosed left parenthesis, ')' expected\n");

    return temp;
}

static inline int get_op_precedence(token_type op){
#ifdef DEBUG
    if(!(0 <= op && op < sizeof(precedence) / sizeof(precedence[0])))
        fatal_printf("Unexpected token_type in get_op_precedence()!\n");
#endif
    if(precedence[op] == 0)
        compile_error_printf("Expected operator\n");
    return precedence[op];
}


ast_node* ast_generate(){
    if(!scanner_next_token(&cur_token))
        return NULL;

    if(cur_token.type == T_PRINT)
        return ast_mknode_print(ast_bin_expr(0));
    else
        compile_error_printf("Unexpected token: '%s'\n", token_to_string(cur_token.type));
    return NULL;
}
