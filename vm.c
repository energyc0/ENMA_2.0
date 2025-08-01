#include "vm.h"
#include "bytecode.h"
#include "hash_table.h"
#include "lang_types.h"
#include "scope.h"
#include "symtable.h"
#include "garbage_collector.h"
#include "utils.h"
#include "parser.h"
#include <stdio.h>
#include <string.h>

static struct virtual_machine vm;

#define VM_STACK_START (vm.stack)
#define VM_STACK_END (VM_STACK_START + sizeof(vm.stack) / sizeof(vm.stack[0]))

static void vm_init();
static void vm_free();
static vm_execute_result vm_execute(struct bytecode_chunk* code);
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
static inline int get_code_line();
static value_t get_variable_value(obj_id_t* id);
static void set_variable_value(obj_id_t* id, value_t value);

//pops two values from the stack and pushes the result
#define CALC_NUMERICAL_OP(return_type, op) do{ \
        value_t b = stack_pop(); \
        value_t a = stack_pop(); \
        if(!IS_NUMBER(a) || !IS_NUMBER(b)) \
            interpret_error_printf(get_code_line(), "Incompatible type for operation. All operands must be numbers!\n");\
        stack_push(return_type(AS_NUMBER(a) op AS_NUMBER(b))); \
    } while(0)

#define CALC_BOOLEAN_OP(op) do{ \
        value_t b = stack_pop(); \
        value_t a = stack_pop(); \
        if(!IS_BOOLEAN(a) || !IS_BOOLEAN(b)) \
            interpret_error_printf(get_code_line(), "Incompatible type for operation. All operands must be booleans!\n");\
        stack_push(VALUE_BOOLEAN(AS_BOOLEAN(a) op AS_BOOLEAN(b))); \
    } while(0)

#define CALC_VAL_OP(return_type, op)

void vm_interpret(){
    vm_init();

    struct bytecode_chunk chunk;
    bcchunk_init(&chunk);

    while(parse_command(&chunk));

    vm_execute(&chunk);
    bcchunk_free(&chunk);

    vm_free();
}

static void vm_init(){
    vm.code = NULL;
    vm.ip = NULL;
    vm.bp = vm.sp = VM_STACK_START;
}


static vm_execute_result vm_execute(struct bytecode_chunk* code){
    //to leave at the end
    bcchunk_write_simple_op(code, OP_RETURN, line_counter);

    vm.code = code;
    vm.ip = vm.code->_code.data;
#ifdef DEBUG
    bcchunk_disassemble("Current bytecode", vm.code);
#endif
    return interpret();
}

static void vm_free(){}

static vm_execute_result interpret(){
    value_t val;
    int is_done = 0;
    while (!is_done) {
        switch (read_byte()) {
            case OP_RETURN: 
                is_done = 1;
                break;
            case OP_PUSH_BP:
                stack_push(VALUE_NUMBER(vm.bp - vm.stack));
                break;
            case OP_POP_BP:
                val = stack_pop();
#ifdef DEBUG
                if(!IS_NUMBER(val))
                    fatal_printf("Stack smashed! Expected BP offset.\n");
#endif          
                vm.bp = &vm.stack[AS_NUMBER(val)];
                break;
            case OP_BP_AS_SP:
                vm.bp = vm.sp;
                break;
            case OP_SP_AS_BP:
                vm.sp = vm.bp;
                break;
            case OP_POP:
                --vm.sp;
#ifdef DEBUG
            if(vm.stack > vm.sp)
                fatal_printf("Stack smashed! Check OP_POP instruction.\n");
#endif
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
            case OP_DEFINE_GLOBAL:{
                value_t expr = stack_pop();
                obj_id_t* id = (obj_id_t*)extract_value(read_constant()).obj;
                if(symtable_get(id, &val) && val.type != VT_NULL)
                    interpret_error_printf(get_code_line(), "'%s' variable redefenition.\n", id->str);
                symtable_set(id, expr);
                break;
            }
            case OP_GET_GLOBAL:{
                obj_string_t* id = (obj_string_t*)(extract_value(read_constant()).obj);
                val = get_variable_value(id);
                stack_push(val);
                break;
            }
            case OP_SET_GLOBAL:{
                obj_string_t* id = (obj_string_t*)(extract_value(read_constant()).obj);
                value_t expr = stack_pop();
                set_variable_value(id, expr);
                stack_push(expr);
                break;
            }
            case OP_DEFINE_LOCAL:{
                fatal_printf("fuck");
                break;
            }
            case OP_GET_LOCAL:{
                int idx = extract_value(read_constant()).number;
                stack_push(vm.bp[idx]);
                break;
            }
            case OP_SET_LOCAL:{
                int idx = extract_value(read_constant()).number;
                vm.bp[idx] = stack_pop();
                break;
            }
            case OP_ADD:{
                value_t b = stack_pop();
                value_t a = stack_pop();
                if(IS_NUMBER(a) && IS_NUMBER(b)){
                    stack_push(VALUE_NUMBER(AS_NUMBER(a) + AS_NUMBER(b)));
                }else if(IS_OBJ(a) && IS_OBJ(b) && IS_OBJSTRING(AS_OBJ(a)) && IS_OBJSTRING(AS_OBJ(b))){
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
                    interpret_error_printf(get_code_line(), "Incompatible types for operation.\n");\
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
                    interpret_error_printf(get_code_line(), "Incompatible types for operation!\n");
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
                    interpret_error_printf(get_code_line(), "Incompatible types for operation!\n");
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
                    interpret_error_printf(get_code_line(), "Incompatible types for operation!\n");
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
    if(vm.sp < VM_STACK_END)
        *vm.sp++ = data;
    else
        fatal_printf("Stack overflow!\n");
#ifdef DEBUG
    printf("After push:\n");
    examine_stack();
#endif
}
static inline value_t stack_pop(){
    if(vm.sp > VM_STACK_START){
#ifdef DEBUG
        --vm.sp;
        printf("After pop:\n");
        examine_stack();
        return *vm.sp;
#else
        return *--vm.sp;
#endif
    }
    fatal_printf("Stack smashed!\n");
}

static inline int get_code_line(){
    //ip is always incremented
    //so it looks at the next instruction so we need -1
    return ((int*)vm.code->_line_data.data)[vm.ip - vm.code->_code.data - 1];
}


static value_t get_variable_value(obj_id_t* id){
    int idx = resolve_local(id);
    if(idx != -1)
        return vm.bp[idx];
    value_t val;
    if(!symtable_get(id, &val) || val.type == VT_NULL)
        interpret_error_printf(get_code_line(), "Undefined identifier %s\n", id->str);
    return val;
}

static void set_variable_value(obj_id_t* id, value_t value){
    int idx = resolve_local(id);
    if(idx != -1){
        vm.bp[idx] = value;
    }else {
        value_t var_val;
        if(!symtable_get(id, &var_val) || var_val.type == VT_NULL)  
            interpret_error_printf(get_code_line(), "Undefined identifier %s\n", id->str);
        if(!is_value_same_type(var_val, value))
            interpret_error_printf(get_code_line(), "Incorrect assignment type for '%s'\n", id->str);
        symtable_set(id, value);
    }
}

#ifdef DEBUG
static char* examine_value(value_t val){
    static char buf[1024];
    switch (val.type) {
        case VT_NUMBER: 
            sprintf(buf, "%d", AS_NUMBER(val));
            return buf;
        case VT_BOOL: 
            return AS_BOOLEAN(val) ? "true" : "false";
        case VT_OBJ:{
            switch (AS_OBJ(val)->type) {
                case OBJ_STRING: return AS_OBJSTRING(val)->str;
                case OBJ_IDENTIFIER:{
                    sprintf(buf, "{%.*s}",(int)(sizeof(buf) - 3), AS_OBJIDENTIFIER(val)->str);
                    return buf;
                }
                default: 
                    fatal_printf("Undefined object type in examine_value()\n");
            }
        }
        case VT_NULL:
            return "NULL - something wrong! Check your code!";
        default: 
            fatal_printf("Undefined value in the stack! Check examine_value()\n");
    }
}


static void examine_stack(){
    printf("=== Stack ===\n");
    for(value_t* ptr = vm.stack; ptr < vm.sp; ptr++)
        printf("%04X | %s\n", (unsigned)(ptr - vm.stack), examine_value(*ptr));
}
#endif