#include "parser.h"
#include "ast.h"
#include "scanner.h"
#include "token.h"
#include "utils.h"

static const int precedence[8] = {0, 1, 1, 2, 2, 0, -1, 0};
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
        case T_INT: return ast_mknode_constant(cur_token.data);
        case T_LPAR: temp = ast_bin_expr(0);  break;
        default:
            compile_error_printf("Expected expression\n");
    }
    if(!is_match(T_RPAR))
        compile_error_printf("Unclosed left parenthesis, ')' expected\n");

    return temp;
}

static inline int get_op_precedence(token_type op){
    if(precedence[op] == 0)
        compile_error_printf("Expected operator\n");
    return precedence[op];
}


ast_node* ast_generate(){
    ast_node* root = ast_bin_expr(0);

    return root;
}
