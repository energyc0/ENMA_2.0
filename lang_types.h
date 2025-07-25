#ifndef LANG_TYPES_H
#define LANG_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef uint8_t byte_t;

typedef enum obj_type{
    OBJ_STRING
}obj_type;

typedef struct obj_t{
    obj_type type;
    struct obj_t* next; // for garbage collector
}obj_t;

typedef struct obj_string_t{
    obj_t obj;
    char* str;
    size_t len;
    int32_t hash;
}obj_string_t;

typedef enum{
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

#define INNERVALUE_AS_NUMBER(value) ((union _inner_value_t){.number = (value)})
#define INNERVALUE_AS_BOOLEAN(value) ((union _inner_value_t){.boolean = (value)})
#define INNERVALUE_AS_OBJ(value) ((union _inner_value_t){.obj = (value)})

#define VALUE_BOOLEAN(value) ((value_t){.type = VT_BOOL, .as = {.boolean = (value)}})
#define VALUE_NUMBER(value) ((value_t){.type = VT_NUMBER, .as = {.number = (value)}})
#define VALUE_OBJ(value) ((value_t){.type = VT_OBJ, .as = {.obj = (obj_t*)(value)}})

#define AS_NUMBER(value) ((value).as.number)
#define AS_BOOLEAN(value) ((value).as.boolean)
#define AS_OBJ(value) ((value).as.obj)
#define AS_OBJSTRING(value) ((obj_string_t*)((value).as.obj))

#define IS_BOOLEAN(value) ((value).type == VT_BOOL)
#define IS_NUMBER(value) ((value).type == VT_NUMBER)
#define IS_OBJ(value) ((value).type == VT_OBJ)

#define IS_OBJSTRING(value) ((value)->type == OBJ_STRING)

//frees obj_t internals and the pointer
void obj_free(obj_t* ptr);
obj_string_t* mk_objstring(char* s, size_t len, int32_t hash);

#endif