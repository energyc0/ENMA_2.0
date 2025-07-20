#include "symtable.h"
#include "data_structs.h"
#include "token.h"
#include <string.h>

#define SYMTABLE_SIZE (1024)

static struct trie_node* keywords = NULL;

static char* symtable[SYMTABLE_SIZE];
static int symtable_size = 0;


void symtable_init(){
    keywords = tr_alloc();
    tr_init(keywords, 0, 0);
    tr_add(keywords, "print", T_PRINT);
}

token_type symtable_procword(char* str){
    struct trie_node* ptr;
    if((ptr = tr_find(keywords, str)) == NULL)
        return T_IDENT;
    else 
        return ptr->data;
}

int symtable_contain(char* id){
    for(int i = 0; i < symtable_size; i++)
        if (strcmp(id, symtable[i]) == 0)
            return 1;
    return 0;
}

int symtable_addident(char* id){
    symtable[symtable_size] = strdup(id);
    return symtable_size++;
}

char* symtable_getident(int idx){
    return symtable[idx];
}