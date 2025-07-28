#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>
#include "token.h"

void scanner_init(FILE* fp);
int scanner_next_token(struct token* t);
//puts token and curent line in the buffer, may be got by scanner_next_token()
void scanner_putback_token();
void scanner_debug_tokens();

#endif