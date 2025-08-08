#include "parser.h"
#include "ast.h"
#include "bytecode.h"
#include "cycler.h"
#include "lang_types.h"
#include "scanner.h"
#include "scope.h"
#include "symtable.h"
#include "token.h"
#include "utils.h"

static const int precedence[] = {
    [T_ADD] = 8,
    [T_SUB] = 8,
    [T_MUL] = 9,
    [T_DIV] = 9,
    [T_AND] = 5,
    [T_OR] = 3,
    [T_XOR] = 4,
    [T_NOT] = 6,
    [T_EQUAL] = 7,
    [T_NEQUAL] = 7,
    [T_GREATER] = 7,
    [T_EGREATER] = 7,
    [T_LESS] = 7,
    [T_ELESS] = 7,
    [T_DECR] = 0,
    [T_INCR] = 0,
    [T_ASSIGN] = 1,
    [T_LPAR] = 0,
    [T_RPAR] = -1,
    [T_INT] = 0,
    [T_FALSE] = 0,
    [T_TRUE] = 0,
    [T_IDENT] = 0,
    [T_STRING] = 0,
    [T_PRINT] = 0,
    [T_VAR] = 0,
    [T_SEMI] = -1,
    [T_LBRACE] = 0,
    [T_RBRACE] = 0,
    [T_IF] = 0,
    [T_ELSE] = 0,
    [T_WHILE] = 0,
    [T_FOR] = 0,
    [T_FUNC] = 0,
    [T_COMMA] = -1,
    [T_EOF] = 0
};

#define PRECEDENCE_ARR_SIZE (sizeof(precedence) / sizeof(int))

extern struct token cur_token;

static inline int get_op_precedence(token_type op);
static ast_node* ast_bin_expr(int prev_precedence);
static ast_node* ast_primary();
static ast_node* ast_call(obj_id_t* id);

static inline void read_block(struct bytecode_chunk* chunk);

static void parse_simple_expression(struct bytecode_chunk* chunk);
static void parse_var(struct bytecode_chunk* chunk);
static void parse_print(struct bytecode_chunk* chunk);
static void parse_if(struct bytecode_chunk* chunk);
static void parse_while(struct bytecode_chunk* chunk);
static void parse_for(struct bytecode_chunk* chunk);

static void parse_func(struct bytecode_chunk* chunk);
static void parse_func_args(obj_function_t* func);
static void parse_func_definition(struct bytecode_chunk* chunk, obj_function_t* func);
static void parse_func_declaration(obj_function_t* func);

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
            return ast_mknode(AST_NOT, AST_DATA_PTR(ast_primary()));
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
        case T_INCR:
            if(!scanner_next_token(&cur_token))
                compile_error_printf("Expected identifier\n");
            return ast_mknode(AST_PREFINCR, AST_DATA_VALUE(VALUE_OBJ(cur_token.data.ptr)));
        case T_DECR:
            if(!scanner_next_token(&cur_token))
                compile_error_printf("Expected identifier\n");
            return ast_mknode(AST_PREFDECR, AST_DATA_VALUE(VALUE_OBJ(cur_token.data.ptr)));
        case T_IDENT:{
            obj_id_t* id = cur_token.data.ptr;
            scanner_next_token(&cur_token);
            if(is_match(T_INCR))
                return ast_mknode(AST_POSTINCR, AST_DATA_VALUE(VALUE_OBJ(id)));
            else if(is_match(T_DECR))
                return ast_mknode(AST_POSTDECR, AST_DATA_VALUE(VALUE_OBJ(id)));
            else if(is_match(T_LPAR))
                return ast_call(id);
            scanner_putback_token();
            return ast_mknode_identifier(id);
        }
        default:
            compile_error_printf("Expected expression\n");
    }
    cur_expect(T_RPAR, "Unclosed left parenthesis, ')' expected\n");

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
    while (scanner_next_token(&cur_token) && !is_match(T_RBRACE)) {
        scanner_putback_token();
        if(!parse_command(chunk))
            break;
    }
    cur_expect(T_RBRACE, "Unclosed statement block, '}' expected\n");
}

ast_node* ast_process_expr(){
    return ast_bin_expr(0);
}

bool parse_command(struct bytecode_chunk* chunk){
    #define IS_GLOBAL_SCOPE(...) do{\
        if(is_global_scope())   \
            compile_error_printf(__VA_ARGS__);\
    }while(0)

    if(!scanner_next_token(&cur_token))
        return false;

    switch(cur_token.type){
        case T_PRINT:{
            IS_GLOBAL_SCOPE("Statement is not expected in global scope.\n");
            parse_print(chunk);
            break;
        }
        case T_LBRACE:{
            IS_GLOBAL_SCOPE("Create new scope is not expected in global scope.\n");
            begin_scope();
            read_block(chunk);
            end_scope(chunk);
            break;
        }
        case T_VAR:{
            parse_var(chunk);
            break;
        }
        case T_IF:{
            IS_GLOBAL_SCOPE("Statement is not expected in global scope.\n");
            parse_if(chunk);
            break;
        }
        case T_WHILE:{
            IS_GLOBAL_SCOPE("Statement is not expected in global scope.\n");
            parse_while(chunk);
            break;
        }
        case T_FOR:{
            IS_GLOBAL_SCOPE("Statement is not expected in global scope.\n");
            parse_for(chunk);
            break;
        }
        case T_BREAK:{
            if(!is_cycle())
                compile_error_printf("Cannot use 'break' outside a loop\n");
            parse_break(chunk, OP_JUMP, line_counter);
            next_expect(T_SEMI, "Expected ';'\n");
            break;
        }
        case T_CONTINUE:{
            if(!is_cycle())
                compile_error_printf("Cannot use 'continue' outside a loop\n");
            parse_continue(chunk, OP_JUMP, line_counter);
            next_expect(T_SEMI, "Expected ';'\n");
            break;
        }
        case T_FUNC:{
            if(!is_global_scope())
                compile_error_printf("Function declaration expected in the global scope\n");
            parse_func(chunk);
            break;
        }
        //it is an expression statement
        default:{
            IS_GLOBAL_SCOPE("Expression is not expected in global scope.\n");
            parse_simple_expression(chunk);
            break;
        }
    }
    return true;
    #undef IS_GLOBAL_SCOPE
}

static void parse_var(struct bytecode_chunk* chunk){
    next_expect(T_IDENT, "Expected identifier\n");

    obj_string_t* var = cur_token.data.ptr;
    if(!declare_variable(var))
        compile_error_printf("'%s' has already defined\n", var->str);

    next_expect(T_ASSIGN, "Expected expression\n");

    ast_node* expr = ast_process_expr();
    if(!define_variable(var, expr, chunk))
        compile_error_printf("'%s' variable redefinition\n", var->str);

    cur_expect(T_SEMI, "Expected ';'\n");
    ast_freenode(expr);
}

static void parse_print(struct bytecode_chunk* chunk){
    ast_node* node = ast_process_expr();
    bcchunk_write_expression(node, chunk, line_counter);
    bcchunk_write_simple_op(chunk, OP_PRINT, line_counter);
    cur_expect(T_SEMI, "Expected ';'\n");
    ast_freenode(node);
}

static void parse_simple_expression(struct bytecode_chunk* chunk){
    scanner_putback_token();
    ast_node* node = ast_process_expr();
    bcchunk_write_expression(node, chunk, line_counter);
    bcchunk_write_simple_op(chunk, OP_POP, line_counter);
    cur_expect(T_SEMI, "Expected ';'\n");
    ast_freenode(node);
}

#define READ_BLOCK(chunk) do{ \
        next_expect(T_LBRACE, "Expected '{'\n"); \
        begin_scope(); \
        read_block(chunk); \
        end_scope(chunk); \
    }while(0)

#define UPDATE_JUMP_LENGTH(chunk, offset) bcchunk_rewrite_constant(chunk, offset, bcchunk_get_codesize(chunk) - offset - sizeof(int))

static void parse_if(struct bytecode_chunk* chunk){
    next_expect(T_LPAR, "Expected '('\n");
    ast_node* log_expr = ast_process_expr();
    bcchunk_write_expression(log_expr, chunk, line_counter);
    cur_expect(T_RPAR, "Expected ')'\n");

    bcchunk_write_simple_op(chunk, OP_FJUMP, line_counter);

    int offset;
    offset = bcchunk_get_codesize(chunk); 
    bcchunk_write_constant(chunk, -(int)sizeof(int), line_counter); 
    READ_BLOCK(chunk);

    if(!scanner_next_token(&cur_token) || !is_match(T_ELSE)){
        UPDATE_JUMP_LENGTH(chunk, offset);
        scanner_putback_token();
        return;
    }

    bcchunk_write_simple_op(chunk, OP_JUMP, line_counter);
    bcchunk_write_constant(chunk, -(int)sizeof(int), line_counter); 
    UPDATE_JUMP_LENGTH(chunk, offset);

    offset = bcchunk_get_codesize(chunk) - sizeof(int); 

    READ_BLOCK(chunk);
    UPDATE_JUMP_LENGTH(chunk, offset);

}

static void parse_while(struct bytecode_chunk* chunk){
    next_expect(T_LPAR, "Expected '('\n");
    int start = bcchunk_get_codesize(chunk);
    ast_node* log_expr = ast_process_expr();
    begin_cycle(chunk);
    bcchunk_write_expression(log_expr, chunk, line_counter);

    bcchunk_write_simple_op(chunk, OP_FJUMP, line_counter);
    int offset = bcchunk_get_codesize(chunk);
    bcchunk_write_constant(chunk, -(int)sizeof(int), line_counter);
    cur_expect(T_RPAR, "Expected ')'\n");

    next_expect(T_LBRACE, "Expected '{'\n");
    read_block(chunk);

    bcchunk_write_simple_op(chunk, OP_JUMP, line_counter);
    bcchunk_write_constant(chunk, start - (bcchunk_get_codesize(chunk) + sizeof(int)), line_counter);

    UPDATE_JUMP_LENGTH(chunk, offset);
    end_cycle(chunk);
}

static void parse_for(struct bytecode_chunk* chunk){
    begin_cycle(chunk);
    next_expect(T_LPAR, "Expected '('\n");
    if(!scanner_next_token(&cur_token))
        compile_error_printf("Expected expression\n");

    switch(cur_token.type){
        case T_VAR: parse_var(chunk); break;
        case T_SEMI: break;
        default:
            scanner_putback_token();
            bcchunk_write_expression(ast_process_expr(), chunk, line_counter);
            break;
    }

    cur_expect(T_SEMI, "Expected ';'\n");

    if(!scanner_next_token(&cur_token))
        compile_error_printf("Expected expression\n");

    int loop_start = bcchunk_get_codesize(chunk);
    switch(cur_token.type){
        case T_SEMI:
            bcchunk_write_simple_op(chunk, OP_BOOLEAN, line_counter);
            bcchunk_write_value(chunk, VALUE_BOOLEAN(true), line_counter);
            break;
        default:
            scanner_putback_token();
            bcchunk_write_expression(ast_process_expr(), chunk, line_counter);
            break;
    }
    bcchunk_write_simple_op(chunk, OP_FJUMP, line_counter);
    int cond_mark = bcchunk_get_codesize(chunk);
    bcchunk_write_constant(chunk, -(int)sizeof(int), line_counter);

    cur_expect(T_SEMI, "Expected ';'\n");

    if(!scanner_next_token(&cur_token))
        compile_error_printf("Expected expression\n");
    
    int postexpr_line = line_counter;
    ast_node* postexpr;
    if(is_match(T_RPAR)){
        postexpr = NULL;
    }else{
        scanner_putback_token();
        postexpr = ast_process_expr();
    }

    cur_expect(T_RPAR, "Expected ')'\n");

    if(!scanner_next_token(&cur_token))
        compile_error_printf("Expected '{' or ';'\n");
    if(is_match(T_LBRACE))
        read_block(chunk);
    else
        cur_expect(T_SEMI, "Expected '{' or ';'\n");

    change_start_offset(bcchunk_get_codesize(chunk));
    if(postexpr){
        bcchunk_write_expression(postexpr, chunk, postexpr_line);
        bcchunk_write_simple_op(chunk, OP_POP, postexpr_line);
    }
    bcchunk_write_simple_op(chunk, OP_JUMP, postexpr_line);
    bcchunk_write_constant(chunk, loop_start - (bcchunk_get_codesize(chunk) + sizeof(int)), postexpr_line);

    UPDATE_JUMP_LENGTH(chunk, cond_mark);
    end_cycle(chunk);
}

static void parse_func(struct bytecode_chunk* chunk){
    next_expect(T_IDENT, "Expected identifier\n");
    obj_function_t* p = mk_objfunc(cur_token.data.ptr);

    next_expect(T_LPAR, "Expected '('\n");
    parse_func_args(p);
    cur_expect(T_RPAR, "Expected ')'\n");

    scanner_next_token(&cur_token);
    if(is_match(T_LBRACE)){
        parse_func_definition(chunk, p);
    }else{
        parse_func_declaration(p);
    }


}

static void parse_func_args(obj_function_t* func){
    scanner_next_token(&cur_token);
    if(!is_match(T_RPAR)){
        do{
            func->args_count++;
        }while(is_match(T_COMMA));
        cur_expect(T_RPAR, "Expected ')'");
    }
}

static void parse_func_definition(struct bytecode_chunk* chunk, obj_function_t* func){
    value_t val;
    if(symtable_get(func->name, &val) && !IS_NULL(val)){
        if(!IS_OBJFUNCTION(val))
            compile_error_printf("'%s' has already defined\n", func->name);
        if(AS_OBJFUNCTION(val)->args_count != func->args_count)
            compile_error_printf("Conflicting with a declaration of '%s' function\n");
        func = AS_OBJFUNCTION(val);
    }
    func->entry_offset = bcchunk_get_codesize(chunk);
    symtable_set(func->name, VALUE_OBJ(func));
    //preamble
    bcchunk_write_simple_op(chunk, OP_PUSH_BP, line_counter);
    bcchunk_write_simple_op(chunk, OP_BP_AS_SP, line_counter);
    begin_scope();
    read_block(chunk);
    end_scope(chunk);
    //postamble
    bcchunk_write_simple_op(chunk, OP_SP_AS_BP, line_counter);
    bcchunk_write_simple_op(chunk, OP_POP_BP, line_counter);
    bcchunk_write_simple_op(chunk, OP_NULL, line_counter);
    bcchunk_write_simple_op(chunk, OP_RETURN, line_counter); // in case if user doesn't write 'return' implicitly
}

static void parse_func_declaration(obj_function_t* func){
    cur_expect(T_SEMI, "Expected ';' or '{'\n");
    func->entry_offset = -1;
    value_t val;
    if(symtable_get(func->name, &val) && !IS_NULL(val)){
        if(!IS_OBJFUNCTION(val))
            compile_error_printf("'%s' has already defined\n", func->name);
        compile_error_printf("'%s' function redefinition\n");
    }
    symtable_set(func->name, VALUE_OBJ(func));

}

static ast_node* ast_call(obj_id_t* id){
    scanner_next_token(&cur_token);
    value_t instance;
    if(!symtable_get(id, &instance))
        compile_error_printf("Undefined identifier '%s'\n", id->str);
    if(!IS_OBJFUNCTION(instance))
        compile_error_printf("'%s' is not a function\n", id->str);

    if(is_match(T_RPAR)){
        return ast_mknode_func(NULL, AS_OBJFUNCTION(instance));
    }else{
        fatal_printf("Unexpected\n");
        do{

        }while(is_match(T_RPAR));   
    }
}
//0x5555555700d0
#undef UPDATE_JUMP_LENGTH
#undef READ_BLOCK