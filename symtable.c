#include "symtable.h"
#include "data_structs.h"
#include "hash_table.h"
#include "lang_types.h"
#include "token.h"
#include <string.h>

/*
#define TABLE_SIZE (1024)
struct table{
    char** data;
    int size;
    int capacity;
};

static inline void table_init(struct table* t);
static inline void table_expand(struct table* t); //expand size
static inline int table_add(struct table* t, char* str);
static inline char* table_get(struct table* t, int idx);
static inline void table_free(struct table* t); //frees 'guts' of the struc
*/

static struct trie_node* keywords = NULL;
static struct hash_table symtable;
static struct hash_table stringtable;

//static obj_string_t* _table_find_string(struct hash_table*t, const char* s, size_t sz, int32_t hash);

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
/*
static obj_string_t* _table_find_string(struct hash_table*t, const char* s, size_t sz, int32_t hash){
    obj_string_t* k = table_find_string(t, s, sz, hash);
    if(k == NULL){
        k = mk_objstring(s, sz, hash);
    }
    return k;
}
    */

obj_string_t* symtable_findstr(const char* s, size_t sz, int32_t hash){
    return table_find_string(&symtable, s, sz, hash);
}
obj_string_t* stringtable_findstr(const char* s, size_t sz, int32_t hash){
    return table_find_string(&stringtable, s, sz, hash);
}


bool symtable_set(obj_string_t* id, value_t val){
    return table_set(&symtable, id, val);
}
bool stringtable_set(obj_string_t* str){
    return table_set(&stringtable, str, VALUE_NULL);
}

bool symtable_check(obj_string_t* id){
    return table_check(&symtable, id, NULL);
}
/*
static inline void table_init(struct table* t){
    t->capacity = TABLE_SIZE;
    t->size = 0;
    t->data = emalloc(t->capacity * sizeof(char*));
}
static inline void table_expand(struct table* t){
    t->capacity += TABLE_SIZE;
    t->data = erealloc(t->data, t->capacity * sizeof(char*));
}
static inline int table_add(struct table* t, char* str){
    if(t->capacity <= t->size)
        table_expand(t);
    t->data[t->size] = strdup(str);
    return t->size++;
}
static inline char* table_get(struct table* t, int idx){
#ifdef DEBUG
    if(!(0 <= idx && idx < t->size))
        fatal_printf("Incorrect index in table_get()!\n");
#endif
    return t->data[idx];
}

static inline void table_free(struct table* t){
    t->capacity = 0;
    t->size = 0;
    free(t->data);
    t->data = NULL;
}*/