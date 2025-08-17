#include "lang_types.h"
#include "utils.h"
#include <stdint.h>
#include <string.h>
#include "garbage_collector.h"
#include "symtable.h"
#include "hash_table.h"
#include "ast.h"

static inline obj_string_t* init_objstr(const char* s, size_t len, int32_t hash, obj_type type){
    obj_string_t* ptr = emalloc(sizeof(obj_string_t));

    ptr->obj.type = type;
    ptr->obj.next = NULL;
    ptr->obj.is_marked = false;
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

obj_function_t* mk_objfunc(obj_string_t* name){
    obj_function_t* ptr = emalloc(sizeof(obj_function_t));
    ptr->base.name = name;
    ptr->base.argc = 0;
    ptr->entry_offset = 0;
    ptr->base.obj.type = OBJ_FUNCTION;
    ptr->base.obj.next = NULL;
    ptr->base.obj.is_marked = false;
    gc_add((obj_t*)ptr);
    return ptr;
}

obj_natfunction_t* mk_objnatfunc(obj_string_t* name, native_function impl){
    obj_natfunction_t* ptr = emalloc(sizeof(obj_natfunction_t));
    ptr->impl = impl;
    ptr->base.argc = 0;
    ptr->base.name = name;
    ptr->base.obj.next = NULL;
    ptr->base.obj.type = OBJ_NATFUNCTION;
    ptr->base.obj.is_marked = false;
    return ptr;
}

obj_class_t* mk_objclass(obj_id_t* name){
    obj_class_t* ptr = emalloc(sizeof(obj_class_t));
    ptr->name = name;
    ptr->fields = mk_table();
    ptr->obj.is_marked = false;
    ptr->obj.next = NULL;
    ptr->obj.type = OBJ_CLASS;
    return ptr;
}

obj_instance_t* mk_objinstance(struct ast_class_info* info){
    obj_instance_t* ptr = emalloc(sizeof(obj_instance_t));
    ptr->impl = info->cl;
    ptr->data = emalloc(sizeof(ptr->data[0]) * ptr->impl->fields->count);
    ptr->obj.is_marked = false;
    ptr->obj.next = NULL;
    ptr->obj.type = OBJ_INSTANCE;
    return ptr;
}

obj_string_t* objstring_conc(value_t a, value_t b){
    obj_string_t* s1 = AS_OBJSTRING(a);
    obj_string_t* s2 = AS_OBJSTRING(b);
    size_t len = s1->len + s2->len;
    char* s = emalloc(len + 1);
    strncpy(s, s1->str, s1->len);
    strncpy(s + s1->len, s2->str, s2->len);
    s[len] = '\0';
    int32_t hash = hash_string(s, len);

    obj_string_t* ptr = stringtable_findstr(s, len, hash);
    if(ptr == NULL)
        ptr = init_objstr(s, len, hash, OBJ_STRING);
    
    return ptr;
}
void obj_free(obj_t* ptr){
    switch (ptr->type) {
        case OBJ_STRING: case OBJ_IDENTIFIER:
            free(((obj_string_t*)ptr)->str);
            break;
        case OBJ_FUNCTION: case OBJ_NATFUNCTION:
            break;
        case OBJ_CLASS:
            table_free(((obj_class_t*)ptr)->fields);
            break;
        case OBJ_INSTANCE:
            free(((obj_instance_t*)ptr)->data);
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

#ifdef DEBUG
const char* get_value_name(value_type type){
    static const char* names[] = {
        [VT_NULL] = "null",
        [VT_NUMBER] = "number",
        [VT_BOOL] = "bool",
        [VT_OBJ] = "obj"
    };
    return names[type];
}

const char* get_obj_name(obj_type type){
    static const char* names[] = {
        [OBJ_STRING] = "string",
        [OBJ_IDENTIFIER] = "identifier",
        [OBJ_FUNCTION] = "function",
        [OBJ_NATFUNCTION] = "native function",
        [OBJ_CLASS] = "class",
        [OBJ_INSTANCE] = "instance"
    };
    return names[type];
}

char* examine_value(value_t val){
    static char buf[1024];
    switch (val.type) {
        case VT_NUMBER: 
            sprintf(buf, "%d", AS_NUMBER(val));
            return buf;
        case VT_BOOL: 
            return AS_BOOLEAN(val) ? "true" : "false";
        case VT_OBJ:{
            switch (AS_OBJ(val)->type) {
                case OBJ_STRING:{
                    sprintf(buf, "\"%.*s\"",(int)(sizeof(buf) - 3), AS_OBJIDENTIFIER(val)->str);
                    return buf;
                }
                case OBJ_IDENTIFIER:{
                    sprintf(buf, "{%.*s}",(int)(sizeof(buf) - 3), AS_OBJIDENTIFIER(val)->str);
                    return buf;
                }
                case OBJ_FUNCTION:{
                    sprintf(buf, "%p %.*s(%d arguments) [0x%X]",
                         AS_OBJFUNCTION(val), (int)(sizeof(buf) - 64),
                          AS_OBJFUNCTION(val)->base.name->str, AS_OBJFUNCTION(val)->base.argc, AS_OBJFUNCTION(val)->entry_offset);
                         return buf;
                }
                case OBJ_NATFUNCTION:{
                    sprintf(buf, "%p %.*s(%d arguments) [native]",
                         AS_OBJNATFUNCTION(val), (int)(sizeof(buf) - 64),
                          AS_OBJNATFUNCTION(val)->base.name->str, AS_OBJNATFUNCTION(val)->base.argc);
                    return buf;
                }
                case OBJ_CLASS:{
                    sprintf(buf, "%p Class %.*s",
                         AS_OBJCLASS(val), (int)(sizeof(buf) - 64),
                          AS_OBJCLASS(val)->name->str);
                    return buf;
                }
                case OBJ_INSTANCE:{
                    sprintf(buf, "%p Instance of class %.*s",
                         AS_OBJINSTANCE(val), (int)(sizeof(buf) - 64),
                          AS_OBJINSTANCE(val)->impl->name->str);
                    return buf;
                }
                default: 
                    fatal_printf("Undefined object type in examine_value()\n");
            }
        }
        case VT_NULL:
            return "NULL";
        default: 
            fatal_printf("Undefined value in the stack! Check examine_value()\n");
    }
}
#endif