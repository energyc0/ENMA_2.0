#ifndef SYMTABLE_H
#define SYMTABLE_H

#include "lang_types.h"
#include "token.h"

void symtable_init();
void symtable_cleanup();

/*return T_IDENT if it is not a keyword and not an identifer,
otherwise return valid token_type*/
token_type symtable_procword(char* str);

//return new allocated string if there is no such, or return existing one
obj_id_t* symtable_findstr(const char* s, size_t sz, int32_t hash);
obj_string_t* stringtable_findstr(const char* s, size_t sz, int32_t hash);

bool symtable_set(obj_id_t* id, value_t val);
bool stringtable_set(obj_string_t* str);

//check if identifier exists
//if true return value
bool symtable_get(const obj_id_t* id, value_t* value);

#endif
