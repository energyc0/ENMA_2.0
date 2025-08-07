#include "hash_table.h"
#include "lang_types.h"
#include "utils.h"
#include <string.h>

#define LOAD_RATIO (0.5)
#define TABLE_BASE_CAPACITY (16)

//find entry in the table
static hash_entry* find_entry(struct hash_table* t, const obj_string_t* key);
//find place in the table to insert new
//static hash_entry* find_place(struct hash_table* t, obj_string_t* key);
//initalize all with zeros
static void entries_init(hash_entry* entries, size_t count);

static void table_grow(struct hash_table*t);

struct hash_table* mk_table(){
    return emalloc(sizeof(struct hash_table));
}

void table_init(struct hash_table* t){
    t->capacity = TABLE_BASE_CAPACITY;
    t->count = 0;
    t->entries = emalloc(sizeof(t->entries[0]) * t->capacity);
    entries_init(t->entries, t->capacity);
}

bool table_set(struct hash_table* t, obj_string_t* key, value_t value){
    if(t->capacity * LOAD_RATIO < t->count +1)
        table_grow(t);
    hash_entry* ptr = find_entry(t, key);
    bool is_new = ptr->key == NULL && !AS_BOOLEAN(ptr->value);
    if(is_new){
        t->count++;
    }
    ptr->key = key;
    ptr->value = value;
    return is_new;
}

bool table_unset(struct hash_table* t, obj_string_t * key){
    hash_entry* ptr = find_entry(t, key);
    if(ptr->key == NULL){
        return false;
    }

    ptr->key = NULL;
    ptr->value = VALUE_BOOLEAN(true);
    return true;
}

bool table_check(struct hash_table* t , const obj_string_t* key, value_t* value){
    hash_entry* ptr = find_entry(t, key);
    if(ptr->key == NULL){
        return false;
    }else{
        if(value != NULL)
            *value = ptr->value;
        return true;
    }
}

void table_free(struct hash_table* t){
    //all keys are freed by garbage collector
    free(t->entries);
}

static hash_entry* find_entry(struct hash_table* t, const obj_string_t* key){
    int32_t idx = key->hash % t->capacity;
    hash_entry* tombstone = NULL;
    for(;;){
        if (t->entries[idx].key == key) {
            return &t->entries[idx];
        }
        if(t->entries[idx].key == NULL){
            if(AS_BOOLEAN(t->entries[idx].value)){
                tombstone = &t->entries[idx];
            }else{
                return tombstone ? tombstone : &t->entries[idx];
            }
        }
        idx = (idx + 1) % t->capacity;
    }
}

int32_t hash_string(const char* str, size_t len){
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < len; i++) {
        hash ^= (uint8_t)str[i];
        hash *= 16777619;
    }
    return hash;
}

obj_string_t* table_find_string(struct hash_table* t, const char* s, size_t sz, int32_t hash){
    int32_t idx = hash % t->capacity;
    for(;;){
        if (t->entries[idx].key && t->entries[idx].key->len == sz && strncmp(t->entries[idx].key->str, s, sz) == 0) {
            return t->entries[idx].key;
        }
        if(t->entries[idx].key == NULL && !AS_BOOLEAN(t->entries[idx].value)){
            return NULL;
        }
        idx = (idx + 1) % t->capacity;
    }
}

static void entries_init(hash_entry* entries, size_t count){
    for(size_t i = 0; i < count; i++){
        entries[i].key = NULL;
        entries[i].value = (typeof(entries[i].value)){};
    }
}

static void table_grow(struct hash_table*t){
    hash_entry* entries = t->entries;
    t->entries = emalloc(t->capacity * 2 * sizeof(t->entries[0]));
    entries_init(t->entries, t->capacity*2);
    for(size_t i = 0; i < t->capacity; i++){
        if (entries[i].key != NULL) {
            table_set(t, entries[i].key, entries[i].value);
        }
    }
    t->capacity *= 2;
}

#ifdef DEBUG
void table_debug(const struct hash_table* t){
    printf("%lu entries:\n", t->count);
    for(size_t i = 0; i < t->capacity; i++){
        if(t->entries[i].key != NULL){
            printf("Entry with key '%s' and value %s\n", t->entries[i].key->str, examine_value(t->entries[i].value));
        }
    }
}

#endif