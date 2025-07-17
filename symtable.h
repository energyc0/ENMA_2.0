#ifndef SYMTABLE_H
#define SYMTABLE_H

#include "token.h"

/*return T_IDENT if it is not a keyword and not an identifer,
otherwise return valid token_type*/
token_type symtable_procword(char* str);

//adds identifier to the table and returns table index
int symtable_addident(char* id);
//returns identifier containing in the table
char* symtable_getident(int idx);

#endif
