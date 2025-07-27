#include "vm.h"
#include "bytecode.h"
#include "hash_table.h"
#include "lang_types.h"
#include "symtable.h"
#include "garbage_collector.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

static struct virtual_machine vm;

#define VM_STACK_START (vm.stack)
#define VM_STACK_END (VM_STACK_START + sizeof(vm.stack) / sizeof(vm.stack[0]))

static vm_execute_result interpret();

#ifdef DEBUG
static char* examine_value(value_t val);
static void examine_stack(); 
#endif

static inline void stack_push(value_t data);
static inline value_t stack_pop();

//reads byte and increases ip pointer
static inline byte_t read_byte();
//reads int
static inline int read_constant();
//extract value from data chunk
static inline union _inner_value_t extract_value(int offset);

//pops two values from the stack and pushes the result
#define CALC_NUMERICAL_OP(return_type, op) do{ \
        value_t b = stack_pop(); \
        value_t a = stack_pop(); \
        if(!IS_NUMBER(a) || !IS_NUMBER(b)) \
            compile_error_printf("Incompatible type for operation. All operands must be numbers!\n");\
        stack_push(return_type(AS_NUMBER(a) op AS_NUMBER(b))); \
    } while(0)

#define CALC_BOOLEAN_OP(op) do{ \
        value_t b = stack_pop(); \
        value_t a = stack_pop(); \
        if(!IS_BOOLEAN(a) || !IS_BOOLEAN(b)) \
            compile_error_printf("Incompatible type for operation. All operands must be booleans!\n");\
        stack_push(VALUE_BOOLEAN(AS_BOOLEAN(a) op AS_BOOLEAN(b))); \
    } while(0)

#define CALC_VAL_OP(return_type, op)

void vm_init(){}

vm_execute_result vm_execute(struct bytecode_chunk* code){
    vm.code = code;
    vm.ip = vm.code->_code.data;
    vm.stack_top = VM_STACK_START;
#ifdef DEBUG
    bcchunk_disassemble("Current bytecode", vm.code);
#endif
    return interpret();
}

void vm_free(){}

static vm_execute_result interpret(){
    value_t val;
    int is_done = 0;
    while (!is_done) {
        switch (read_byte()) {
            case OP_RETURN: 
                is_done = 1;
                break;
            case OP_NUMBER:
                stack_push(VALUE_NUMBER(extract_value(read_constant()).number));
                break;
            case OP_BOOLEAN:
                stack_push(VALUE_BOOLEAN(extract_value(read_constant()).boolean));
                break;
            case OP_STRING:
                stack_push(VALUE_OBJ(extract_value(read_constant()).obj));
                break;
            case OP_ADD:{
                value_t b = stack_pop();
                value_t a = stack_pop();
                if(IS_NUMBER(a) && IS_NUMBER(b)){
                    stack_push(VALUE_NUMBER(AS_NUMBER(a) + AS_NUMBER(b)));
                }else if(IS_OBJSTRING(AS_OBJ(a)) && IS_OBJSTRING(AS_OBJ(b))){
                    obj_string_t* s1 = AS_OBJSTRING(a);
                    obj_string_t* s2 = AS_OBJSTRING(b);
                    size_t len = s1->len + s2->len;
                    char* s = emalloc(len + 1);
                    strncpy(s, s1->str, s1->len);
                    strncpy(s + s1->len, s2->str, s2->len);
                    s[len] = '\0';
                    int32_t hash = hash_string(s, len);

                    obj_string_t*ptr = stringtable_findstr(s, len, hash);
                    if(ptr == NULL){
                        ptr = emalloc(sizeof(obj_string_t));
                        ptr->obj.type = OBJ_STRING;
                        ptr->obj.next = NULL;
                        ptr->hash = hash;
                        ptr->len = len;
                        ptr->str = s;
                        //add it to the garbage collector
                        gc_add((obj_t*)ptr);
                    }
                    stack_push(VALUE_OBJ(ptr));
                }else{
                    compile_error_printf("Incompatible types for operation.\n");\
                }
            }
                break;
            case OP_SUB: 
                CALC_NUMERICAL_OP(VALUE_NUMBER,-);
                break;
            case OP_DIV: 
                CALC_NUMERICAL_OP(VALUE_NUMBER,/);
                break;
            case OP_MUL: 
                CALC_NUMERICAL_OP(VALUE_NUMBER,*);
                break; 
            case OP_AND:
                CALC_BOOLEAN_OP(&&);
                break;
            case OP_OR:
                CALC_BOOLEAN_OP(||);
                break;
            case OP_XOR:
                CALC_BOOLEAN_OP(^);
                break;
            case OP_NOT:
                stack_push(VALUE_BOOLEAN(!AS_BOOLEAN(stack_pop())));
                break;
            case OP_EQUAL:{
                value_t b = stack_pop();
                value_t a = stack_pop();
                if(IS_NUMBER(a) && IS_NUMBER(b)){
                    stack_push(VALUE_BOOLEAN(AS_NUMBER(a) == AS_NUMBER(b)));
                }else if(IS_BOOLEAN(a) && IS_BOOLEAN(b)){
                    stack_push(VALUE_BOOLEAN(AS_BOOLEAN(a) == AS_BOOLEAN(b)));
                }else if(IS_OBJSTRING(AS_OBJ(a)) && IS_OBJSTRING(AS_OBJ(b))){
                    stack_push(VALUE_BOOLEAN(AS_OBJSTRING(a) == AS_OBJSTRING(b)));
                }else{
                    compile_error_printf("Incompatible types for operation!\n");
                }
                break;
            }
            case OP_GREATER:{
                /*TODO: add strings compare*/
                value_t b = stack_pop();
                value_t a = stack_pop();
                if(IS_NUMBER(a) && IS_NUMBER(b)){
                    stack_push(VALUE_BOOLEAN(AS_NUMBER(a) > AS_NUMBER(b)));
                }else if(IS_BOOLEAN(a) && IS_BOOLEAN(b)){
                    stack_push(VALUE_BOOLEAN(AS_BOOLEAN(a) > AS_BOOLEAN(b)));
                }else{
                    compile_error_printf("Incompatible types for operation!\n");
                }
                break;
            }
            case OP_LESS:{
                /*TODO: add strings compare*/
                value_t b = stack_pop();
                value_t a = stack_pop();
                if(IS_NUMBER(a) && IS_NUMBER(b)){
                    stack_push(VALUE_BOOLEAN(AS_NUMBER(a) < AS_NUMBER(b)));
                }else if(IS_BOOLEAN(a) && IS_BOOLEAN(b)){
                    stack_push(VALUE_BOOLEAN(AS_BOOLEAN(a) < AS_BOOLEAN(b)));
                }else{
                    compile_error_printf("Incompatible types for operation!\n");
                }
                break;
            }
            case OP_PRINT:
                val = stack_pop();
                if(IS_NUMBER(val)){
                    printf("%d\n", AS_NUMBER(val));
                }else if(IS_BOOLEAN(val)){
                    printf("%s\n", AS_BOOLEAN(val) ? "true" : "false");
                }else if(IS_OBJ(val)){
                    switch (AS_OBJ(val)->type) {
                        case OBJ_STRING: printf("%s\n", AS_OBJSTRING(val)->str); break;
                        default: fatal_printf("Undefined obj_type in interpret()!\n");
                    }
                }else{
                    printf("print: Not implemented instruction :(\n");
                }
                break;
            default: 
                eprintf("Undefined instruction!\n");
                return VME_RUNTIME_ERROR;
        }
    }
    return VME_SUCCESS;
}

static inline byte_t read_byte(){
    return (*vm.ip++);
}

static inline int read_constant(){
    int num = *(int*)vm.ip;
    vm.ip+=sizeof(int);
    return num;
}

static inline union _inner_value_t extract_value(int offset){
    return *(union _inner_value_t*)(vm.code->_data.data + offset);
}

static inline void stack_push(value_t data){
    if(vm.stack_top < VM_STACK_END)
        *vm.stack_top++ = data;
    else
        fatal_printf("Stack overflow!\n");
#ifdef DEBUG
    printf("After push:\n");
    examine_stack();
#endif
}
static inline value_t stack_pop(){
    if(vm.stack_top > VM_STACK_START){
#ifdef DEBUG
        --vm.stack_top;
        printf("After pop:\n");
        examine_stack();
        return *vm.stack_top;
#else
        return *--vm.stack_top;
#endif
    }
    fatal_printf("Stack smashed!\n");
}

#ifdef DEBUG
static char* examine_value(value_t val){
    static char buf[16];
    switch (val.type) {
        case VT_NUMBER: 
            sprintf(buf, "%d", AS_NUMBER(val));
            return buf;
        case VT_BOOL: 
            return AS_BOOLEAN(val) ? "true" : "false";
        case VT_OBJ:{
            switch (AS_OBJ(val)->type) {
                case OBJ_STRING: return AS_OBJSTRING(val)->str;
                default: 
                    fatal_printf("Undefined object type in examine_value()\n");
            }
        }
        default: 
            fatal_printf("Undefined value in the stack! Check examine_value()\n");
    }
}


static void examine_stack(){
    printf("=== Stack ===\n");
    for(value_t* ptr = vm.stack; ptr < vm.stack_top; ptr++)
        printf("%04X | %s\n", (unsigned)(ptr - vm.stack), examine_value(*ptr));
}
#endif
