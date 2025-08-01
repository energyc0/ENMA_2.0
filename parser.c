#include "parser.h"
#include "ast.h"
#include "bytecode.h"
#include "lang_types.h"
#include "scanner.h"
#include "scope.h"
#include "symtable.h"
#include "token.h"
#include "utils.h"

static const int precedence[] = {
    8,  //T_ADD
    8,  //T_SUB
    9,  //T_MUL
    9,  //T_DIV
    5,  //T_AND
    3,  //T_OR
    4,  //T_XOR
    6,  //T_NOT
    7,  //T_EQUAL,
    7,  //T_NEQUAL,
    7,  //T_GREATER,
    7,  //T_EGREATER,
    7,  //T_LESS, 
    7,  //T_ELESS,
    1,  //T_ASSIGN,
    0,  //T_LPAR
    -1, //T_RPAR
    0,  //T_T_INT
    0,  //T_FALSE
    0,  //T_TRUE
    0,  //T_IDENT
    0,  //T_STRING
    0,  //T_PRINT
    0,  //T_VAR
    -1, //T_SEMI
    0,  //T_LBRACE
    0,  //T_RBRACE
    0   //T_EOF
};

#define PRECEDENCE_ARR_SIZE (sizeof(precedence) / sizeof(int))

extern struct token cur_token;

static inline int get_op_precedence(token_type op);
static ast_node* ast_bin_expr(int prev_precedence);
static ast_node* ast_primary();

static inline void read_block(struct bytecode_chunk* chunk);
//parse expressions after 'var'
static void parse_var(struct bytecode_chunk* chunk);

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
            return ast_mknode_number(cur_token.data.num);
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
            return ast_mknode_string(cur_token.data.ptr);
        case T_SUB:
            temp = ast_primary();
            if(temp->type == AST_NUMBER){
                AS_NUMBER(temp->data.val) *= -1; 
                return temp;
            }else{
                return ast_mknode_binary(AST_MUL, ast_mknode_number(-1), temp);
            }
        case T_IDENT:
            return ast_mknode_identifier(cur_token.data.ptr);
        default:
            compile_error_printf("Expected expression\n");
    }
    if(!is_match(T_RPAR))
        compile_error_printf("Unclosed left parenthesis, ')' expected\n");

    return temp;
}

static inline int get_op_precedence(token_type op){
#ifdef DEBUG
    if(PRECEDENCE_ARR_SIZE != T_EOF + 1)
        fatal_printf("precedence[] doesn't contain all tokens! Update it.\n");
    if(!(0 <= op && op < sizeof(precedence) / sizeof(precedence[0])))
        fatal_printf("Unexpected token_type in get_op_precedence()!\n");
#endif
    if(precedence[op] == 0)
        compile_error_printf("Expected operator\n");
    return precedence[op];
}

static inline void read_block(struct bytecode_chunk* chunk){
    while (scanner_next_token(&cur_token) && cur_token.type != T_RBRACE) {
        scanner_putback_token();
        if(!parse_command(chunk))
            break;
    }
    if(cur_token.type != T_RBRACE)
        compile_error_printf("Unclosed statement block, '}' expected\n");
}

ast_node* ast_process_expr(){
    return ast_bin_expr(0);
}

bool parse_command(struct bytecode_chunk* chunk){
    if(!scanner_next_token(&cur_token))
        return false;

    ast_node* node;
    switch(cur_token.type){
        case T_PRINT:{
            node = ast_process_expr();
            bcchunk_write_expression(node, chunk, line_counter);
            bcchunk_write_simple_op(chunk, OP_PRINT, line_counter);
            break;
        }
        case T_LBRACE:{
            begin_scope(chunk);
            read_block(chunk);
            end_scope(chunk);
            return true;
        }
        case T_VAR:{
            parse_var(chunk);
            break;
        }
        //it is an expression statement
        default:{
            scanner_putback_token();
            node = ast_process_expr();
            bcchunk_write_expression(node, chunk, line_counter);
            bcchunk_write_simple_op(chunk, OP_POP, line_counter);
            break;
        }
    }
    if(!is_match(T_SEMI)) 
        compile_error_printf("Expected ';'\n");
    return true;
}

void parse_var(struct bytecode_chunk* chunk){
    if(!scanner_next_token(&cur_token) || !is_match(T_IDENT))
        compile_error_printf("Expected identifier\n");

    obj_string_t* var = cur_token.data.ptr;
    if(!declare_variable(var))
        compile_error_printf("'%s' has already defined\n", var->str);

    if (!scanner_next_token(&cur_token) || !is_match(T_ASSIGN))
        compile_error_printf("Expected expression\n");

    ast_node* expr = ast_process_expr();
    if(!define_variable(var, expr, chunk))
        compile_error_printf("'%s' variable redefinition\n", var->str);
}