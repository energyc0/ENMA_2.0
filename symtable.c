#include "symtable.h"
#include <string.h>

#define SYMTABLE_SIZE (1024)

static char* symtable[SYMTABLE_SIZE];
static int symtable_size = 0;

token_type symtable_procword(char* str){
    if(strcmp("print", str) == 0)
        return T_PRINT;
    return T_IDENT;
}

int symtable_addident(char* id){
    symtable[symtable_size] = strdup(id);
    return symtable_size++;
}

char* symtable_getident(int idx){
    return symtable[idx];
}