#include "symtable.h"
#include "data_structs.h"
#include "hash_table.h"
#include "lang_types.h"
#include "token.h"
#include <string.h>
#include <stdio.h>


static struct trie_node* keywords = NULL;
static struct hash_table symtable;
static struct hash_table stringtable;

void symtable_init(){
    keywords = tr_alloc();
    tr_init(keywords, 0, 0);
    tr_add(keywords, "print", T_PRINT);
    tr_add(keywords,"false", T_FALSE);
    tr_add(keywords,"true", T_TRUE);
    tr_add(keywords, "and", T_AND);
    tr_add(keywords, "or", T_OR);
    tr_add(keywords, "xor", T_XOR);
    tr_add(keywords, "not", T_NOT);
    tr_add(keywords, "var", T_VAR);
    tr_add(keywords, "if", T_IF);
    tr_add(keywords, "else", T_ELSE);
    tr_add(keywords, "while", T_WHILE);
    tr_add(keywords, "for", T_FOR);
    tr_add(keywords, "break", T_BREAK);
    tr_add(keywords, "continue", T_CONTINUE);
    tr_add(keywords, "func", T_FUNC);

    table_init(&symtable);
    table_init(&stringtable);
}

void symtable_cleanup(){
    tr_free(keywords);
    table_free(&symtable);
    table_free(&stringtable);
}

token_type symtable_procword(char* str){
    struct trie_node* ptr;
    if((ptr = tr_find(keywords, str)) == NULL)
        return T_IDENT;
    else 
        return ptr->data;
}

obj_id_t* symtable_findstr(const char* s, size_t sz, int32_t hash){
    return table_find_string(&symtable, s, sz, hash);
}
obj_string_t* stringtable_findstr(const char* s, size_t sz, int32_t hash){
    return table_find_string(&stringtable, s, sz, hash);
}

bool symtable_set(obj_id_t* id, value_t val){
    return table_set(&symtable, id, val);
}
bool stringtable_set(obj_string_t* str){
    return table_set(&stringtable, str, VALUE_NULL);
}

bool symtable_get(const obj_id_t* id, value_t* value){
    return table_check(&symtable, id, value);
}

#ifdef DEBUG
void symtable_debug(){
    printf("Symtable with ");
    table_debug(&symtable);
}

void stringtable_debug(){
    printf("Stringtable with ");
    table_debug(&stringtable);
}

#endif