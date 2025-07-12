#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>
#include "token.h"

void scanner_init(FILE* fp);
int scanner_next_token(struct token* t);
void scanner_debug_tokens();

#endif