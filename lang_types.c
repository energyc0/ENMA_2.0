#include "lang_types.h"
#include "utils.h"
#include <stdint.h>
#include <string.h>
#include "garbage_collector.h"

static inline obj_string_t* init_objstr(const char* s, size_t len, int32_t hash, obj_type type){
    obj_string_t* ptr = emalloc(sizeof(obj_string_t));
    
    ptr->obj.type = type;
    ptr->obj.next = NULL;
    ptr->hash = hash;
    ptr->len = len;
    ptr->str = emalloc(len + 1);
    strncpy(ptr->str, s, len);
    ptr->str[len]='\0';
    //add it to the garbage collector
    gc_add((obj_t*)ptr);
    return ptr;
}

obj_string_t* mk_objstring(const char* s, size_t len, int32_t hash){
    return init_objstr(s, len, hash, OBJ_STRING);
}

obj_id_t* mk_objid(const char* s, size_t len, int32_t hash){
    return init_objstr(s, len, hash, OBJ_IDENTIFIER);
}

obj_string_t* objstring_conc(const obj_string_t* s1, const obj_string_t* s2){
    obj_string_t* ptr = emalloc(sizeof(obj_string_t));
    
    ptr->obj.type = OBJ_STRING;
    ptr->obj.next = NULL;
    ptr->len = s1->len + s2->len;
    ptr->str = emalloc(ptr->len + 1);
    strncpy(ptr->str, s1->str, s1->len);
    strncpy(ptr->str + s1->len, s2->str, s2->len);
    ptr->str[ptr->len]='\0';
    //add it to the garbage collector
    gc_add((obj_t*)ptr);
    return ptr;
}
void obj_free(obj_t* ptr){
    switch (ptr->type) {
        case OBJ_STRING: case OBJ_IDENTIFIER:
            free(((obj_string_t*)ptr)->str);
            break;
        default:
            fatal_printf("Undefined obj_t in obj_free()\n");
    }
    free(ptr);
}

bool is_value_same_type(const value_t a, const value_t b){
    if(a.type != b.type)
        return false;
    if(a.type == VT_OBJ){
        return AS_OBJ(a)->type == AS_OBJ(b)->type;
    }
    return true;
}

bool is_equal_objstring(const obj_string_t* s1, const obj_string_t* s2){
    return s1->len == s2->len && strncmp(s1->str, s2->str, s1->len) == 0;
}