#include "token.h"
#include "utils.h"

const char* token_to_string(token_type type){
    static const char* tok_strs[] = {
        "EOF",
        "+",
        "-",
        "*",
        "/",
        "(",
        ")",
        "INTEGER"
    };
#ifdef DEBUG
    if(!(0 <= type && type <= sizeof(tok_strs) / sizeof(tok_strs[0])))
        fatal_printf("Undefined token type in token_to_string()\n");
#endif
    return tok_strs[type];
}