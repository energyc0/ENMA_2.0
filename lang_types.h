#ifndef LANG_TYPES_H
#define LANG_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef uint8_t byte_t;
typedef struct obj_t obj_t;

typedef enum{
    VT_NONE,    //value is initialized this first
    VT_UNINIT,  //for class fields
    VT_BOOL,
    VT_NUMBER,
    VT_OBJ
}value_type;

union _inner_value_t{
    bool boolean;
    int number;
    obj_t* obj;
};

typedef struct{
    value_type type;
    union _inner_value_t as;
}value_t;

typedef enum obj_type{
    OBJ_STRING,    
    OBJ_IDENTIFIER,
    OBJ_FUNCTION,
    OBJ_NATFUNCTION, //native function
    OBJ_CLASS,
    OBJ_INSTANCE
}obj_type;

typedef struct obj_t{
    obj_type type;
    bool is_marked;
    struct obj_t* next; // for garbage collector
}obj_t;

typedef struct obj_string_t{
    obj_t obj;
    char* str;
    size_t len;
    int32_t hash;
}obj_string_t;

typedef struct obj_string_t obj_id_t;

typedef struct obj_func_base_t{
    obj_t obj;
    int argc;
    obj_string_t* name;
}obj_func_base_t;

typedef struct obj_function_t{
    obj_func_base_t base;
    int entry_offset;
}obj_function_t;

//argc and argv
typedef value_t (*native_function)(int, value_t*);

typedef struct obj_natfunction_t{
    obj_func_base_t base;
    native_function impl;
}obj_natfunction_t;

/*
    obj_class_t has hash_table that contains keys as fields and values as offsets in obj_instance_t
    obj_instance_t contains data that is dynamically allocated and contains data offsets according to hash_table in obj_class_t
*/
typedef value_t field_t;
struct hash_table;

#define CONSTRUCTORS_LIMIT (16)

typedef struct obj_class_t{
    obj_t obj;
    obj_id_t* name;
    struct hash_table* fields;
    struct hash_table* methods;
    obj_function_t* constructors[CONSTRUCTORS_LIMIT + 1]; // for default constructor
}obj_class_t;

typedef struct obj_instance_t{
    obj_t obj;
    obj_class_t* impl;
    field_t* data;
}obj_instance_t;

#define INNERVALUE_AS_NUMBER(value) ((union _inner_value_t){.number = (value)})
#define INNERVALUE_AS_BOOLEAN(value) ((union _inner_value_t){.boolean = (value)})
#define INNERVALUE_AS_OBJ(value) ((union _inner_value_t){.obj = (value)})

#define VALUE_BOOLEAN(value) ((value_t){.type = VT_BOOL, .as = {.boolean = (value)}})
#define VALUE_NUMBER(value) ((value_t){.type = VT_NUMBER, .as = {.number = (value)}})
#define VALUE_OBJ(value) ((value_t){.type = VT_OBJ, .as = {.obj = (obj_t*)(value)}})
#define VALUE_NONE ((value_t){.type = VT_NONE})
#define VALUE_UNINIT ((value_t){.type = VT_UNINIT})

#define AS_NUMBER(value) ((value).as.number)
#define AS_BOOLEAN(value) ((value).as.boolean)
#define AS_OBJ(value) ((value).as.obj)
#define AS_OBJSTRING(value) ((obj_string_t*)AS_OBJ(value))
#define AS_OBJIDENTIFIER(value) ((obj_id_t*)AS_OBJ(value))
#define AS_OBJFUNCBASE(value) ((obj_func_base_t*)AS_OBJ(value))
#define AS_OBJFUNCTION(value) ((obj_function_t*)AS_OBJ(value))
#define AS_OBJNATFUNCTION(value) ((obj_natfunction_t*)AS_OBJ(value))
#define AS_OBJCLASS(value) ((obj_class_t*)AS_OBJ(value))
#define AS_OBJINSTANCE(value) ((obj_instance_t*)AS_OBJ(value))

#define IS_BOOLEAN(value) ((value).type == VT_BOOL)
#define IS_NUMBER(value) ((value).type == VT_NUMBER)
#define IS_OBJ(value) ((value).type == VT_OBJ)
#define IS_NONE(value) ((value).type == VT_NONE)
#define IS_UNINIT(value) ((value).type == VT_UNINIT)
#define IS_EMPTY(value) ({value_t t = (value).type; t == VT_UNINIT || t == VT_NONE})

#define IS_OBJSTRING(value) (IS_OBJ(value) && AS_OBJ(value)->type == OBJ_STRING)
#define IS_OBJIDENTIFIER(value) (IS_OBJ(value) && AS_OBJ(value)->type == OBJ_IDENTIFIER)
#define IS_OBJFUNC(value) (IS_OBJ(value) && (AS_OBJ(value)->type == OBJ_FUNCTION || AS_OBJ(value)->type == OBJ_NATFUNCTION))
#define IS_OBJFUNCTION(value) (IS_OBJ(value) && AS_OBJ(value)->type == OBJ_FUNCTION)
#define IS_OBJNATFUNCTION(value) (IS_OBJ(value) && AS_OBJ(value)->type == OBJ_NATFUNCTION)
#define IS_OBJCLASS(value) (IS_OBJ(value) && AS_OBJ(value)->type == OBJ_CLASS)
#define IS_OBJINSTANCE(value) (IS_OBJ(value) && AS_OBJ(value)->type == OBJ_INSTANCE)

struct ast_class_info;

//frees obj_t internals and the pointer
void obj_free(obj_t* ptr);
obj_string_t* mk_objstring(const char* s, size_t len, int32_t hash);
obj_id_t* mk_objid(const char* s, size_t len, int32_t hash);
obj_function_t* mk_objfunc(obj_string_t* name);
obj_natfunction_t* mk_objnatfunc(obj_string_t* name, native_function impl);
obj_class_t* mk_objclass(obj_id_t* name);
obj_instance_t* mk_objinstance(obj_class_t* cl);

obj_string_t* objstring_conc(value_t a, value_t b);

bool is_equal_objstring(const obj_string_t* s1, const obj_string_t* s2);

bool is_value_same_type(const value_t a, const value_t b);

void set_constructor(obj_class_t* cl, obj_function_t* f);
obj_function_t* find_constructor(obj_class_t* cl, int argc);
void add_ancestor(obj_class_t* cl, obj_id_t* id);

#ifdef DEBUG
const char* get_value_name(value_type type);
const char* get_obj_name(obj_type type);
void examine_value(value_t val);
void print_value(int offset, value_t val);
#endif

#endif