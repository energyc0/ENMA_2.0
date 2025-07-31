#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "token.h"

struct bytecode_chunk;

//generates expression to interpret
ast_node* ast_process_expr();

bool parse_command(struct bytecode_chunk* chunk);

#endif