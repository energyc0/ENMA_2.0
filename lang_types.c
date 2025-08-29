#include "lang_types.h"
#include "utils.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "garbage_collector.h"
#include "symtable.h"
#include "hash_table.h"

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
    ptr->entry_offset = -1;
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
    table_init(ptr->fields);
    ptr->methods = mk_table();
    table_init(ptr->methods);
    ptr->obj.is_marked = false;
    ptr->obj.next = NULL;
    ptr->obj.type = OBJ_CLASS;
    for(int i = 0;i < CONSTRUCTORS_LIMIT; i++)
        ptr->constructors[i] = NULL;
    return ptr;
}

obj_instance_t* mk_objinstance(obj_class_t* cl){
    obj_instance_t* ptr = emalloc(sizeof(obj_instance_t));
    ptr->impl = cl;
    ptr->data = emalloc(sizeof(ptr->data[0]) * ptr->impl->fields->count);
    for(size_t i = 0; i < ptr->impl->fields->count; i++)
        ptr->data[i] = VALUE_UNINIT;
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
            table_free(((obj_class_t*)ptr)->methods);
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

void set_constructor(obj_class_t* cl, obj_function_t* f){
    int i = 0;
    for(;i < CONSTRUCTORS_LIMIT && cl->constructors[i] != NULL; i++){
        if(cl->constructors[i]->base.argc == f->base.argc)
            compile_error_printf("Class '%s' already has a constructor with %d arguments\n",cl->name->str, f->base.argc);
    }
    if(i < CONSTRUCTORS_LIMIT || f->base.argc == 0) // for default constructor
        cl->constructors[i] = f;
    else
        compile_error_printf("Constructors limit exceeded\n");
}

obj_function_t* find_constructor(obj_class_t* cl, int argc){
    for(int i = 0; i <= CONSTRUCTORS_LIMIT && cl->constructors[i] != NULL; i++) // <= because of default constructor
        if(cl->constructors[i]->base.argc == argc)
            return cl->constructors[i];
    return NULL;
}

#ifdef DEBUG
const char* get_value_name(value_type type){
    static const char* names[] = {
        [VT_NONE] = "nothing",
        [VT_UNINIT] = "null",
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

void examine_value(value_t val){
    switch (val.type) {
        case VT_NUMBER: 
            printf("%d", AS_NUMBER(val));
            break;
        case VT_BOOL: 
            printf(AS_BOOLEAN(val) ? "true" : "false");
            break;
        case VT_OBJ:{
            switch (AS_OBJ(val)->type) {
                case OBJ_STRING:{
                    printf("\"%s\"",AS_OBJIDENTIFIER(val)->str);
                    break;
                }
                case OBJ_IDENTIFIER:{
                    printf("{%s}",(AS_OBJIDENTIFIER(val)->str));
                    break;
                }
                case OBJ_FUNCTION:{
                    printf("%p %s(%d arguments) [0x%X]",
                    AS_OBJFUNCTION(val),
                    AS_OBJFUNCTION(val)->base.name->str,
                    AS_OBJFUNCTION(val)->base.argc,
                    AS_OBJFUNCTION(val)->entry_offset);
                    break;
                }
                case OBJ_NATFUNCTION:{
                    printf("%p %s(%d arguments) [native]",
                    AS_OBJNATFUNCTION(val),
                    AS_OBJNATFUNCTION(val)->base.name->str,
                    AS_OBJNATFUNCTION(val)->base.argc);
                    break;
                }
                case OBJ_CLASS:{
                    obj_class_t* p = AS_OBJCLASS(val);
                    printf("%p Class %s\n",p , p->name->str);
                    printf("Constructors:\n\t[");
                    int len = ARR_SIZE(p->constructors);
                    for(int i = 0; i < len && p->constructors[i] != NULL; i++){
                        examine_value(VALUE_OBJ(p->constructors[i]));
                        if(i + 1 < len && p->constructors[i+1] != NULL)
                            printf(", ");
                    }
                    printf("]\n");
                    printf("Fields: \n\t[");
                    if(p->fields->count > 0)
                        table_debug(p->fields);
                    putchar(']');
                    break;
                }
                case OBJ_INSTANCE:{
                    obj_instance_t* p = AS_OBJINSTANCE(val);
                    printf("%p Instance of class %s with properties:\n\t[", p,p->impl->name->str);
                    int data_count = p->impl->fields->count;    
                    for(int i = 0; i < data_count; i++){
                        examine_value(p->data[i]);
                        if(i + 1 < data_count)
                            printf(", ");
                    }
                    putchar(']');
                    break;
                }
                default: 
                    fatal_printf("Undefined object type in examine_value()\n");
            }
            break;
        }
        case VT_NONE:
            printf("NONE");
            break;
        case VT_UNINIT:
            printf("UNINIT");
            break;
        default: 
            fatal_printf("Undefined value in the stack! Check examine_value()\n");
    }
}

void print_value(int offset, value_t val){
    printf("\t%04X | ", offset);
    examine_value(val);
    putchar('\n');
}

#endif