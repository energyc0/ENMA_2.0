#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "lang_types.h"

typedef struct hash_entry{
    obj_string_t* key;
    value_t value;
}hash_entry;

struct hash_table{
    hash_entry* entries;
    size_t count;
    size_t capacity;
};

//dynamically allocate hash table
struct hash_table* mk_table();
//initalize all the data
void table_init(struct hash_table* t);
//return true if new entry was added
bool table_set(struct hash_table* t, obj_string_t* key, value_t value);
//return true if the entry is in the table
//and write its value
bool table_check(struct hash_table* t , const obj_string_t* key, value_t* value);
//return true if the entry was successfully deleted
bool table_unset(struct hash_table* t, obj_string_t * key);

obj_string_t* table_find_string(struct hash_table* t, const char* s, size_t sz, int32_t hash);

void table_free(struct hash_table* t);

int32_t hash_string(const char* str, size_t len);

#endif