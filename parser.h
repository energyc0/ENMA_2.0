#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "token.h"

//generates one statement to interpret
//return NULL on EOF
ast_node* ast_generate();

#endif