#include "lang_types.h"
#include "utils.h"
#include <string.h>
#include "garbage_collector.h"

obj_string_t* mk_objstring(char* s, size_t len){
    obj_string_t* ptr = emalloc(sizeof(obj_string_t));
    
    ptr->obj.type = OBJ_STRING;
    ptr->obj.next = NULL;
    
    ptr->len = len;
    ptr->str = emalloc(len + 1);
    strncpy(ptr->str, s, len);
    ptr->str[len]='\0';

    gc_add((obj_t*)ptr);
    return ptr;
}

void obj_free(obj_t* ptr){
    switch (ptr->type) {
        case OBJ_STRING:
            free(((obj_string_t*)ptr)->str);
            break;
        default:
            fatal_printf("Undefined obj_t in obj_free()\n");
    }
    free(ptr);
}