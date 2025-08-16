#include "vm.h"
#include "bytecode.h"
#include "hash_table.h"
#include "lang_types.h"
#include "scope.h"
#include "symtable.h"
#include "utils.h"
#include "parser.h"
#include <stdio.h>
#include <string.h>
#include "garbage_collector.h"

struct virtual_machine vm;

#define VM_STACK_START (vm.stack)
#define VM_STACK_END (VM_STACK_START + sizeof(vm.stack) / sizeof(vm.stack[0]))
#define ENTRY_FUNCTION_NAME "main"

static void vm_init();
static void vm_free();
static vm_execute_result vm_execute(struct bytecode_chunk* code);
static vm_execute_result interpret();

#ifdef DEBUG
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
static value_t get_variable_value(obj_id_t* id);
static void set_variable_value(obj_id_t* id, value_t value);

static void preamble();
static void epilogue();

//pops two values from the stack and pushes the result
#define CALC_NUMERICAL_OP(return_type, op) do{ \
        value_t b = stack_pop(); \
        value_t a = stack_pop(); \
        if(!IS_NUMBER(a) || !IS_NUMBER(b)) \
            interpret_error_printf(get_vm_codeline(), "Incompatible type for operation. All operands must be numbers!\n");\
        stack_push(return_type(AS_NUMBER(a) op AS_NUMBER(b))); \
    } while(0)

#define CALC_BOOLEAN_OP(op) do{ \
        value_t b = stack_pop(); \
        value_t a = stack_pop(); \
        if(!IS_BOOLEAN(a) || !IS_BOOLEAN(b)) \
            interpret_error_printf(get_vm_codeline(), "Incompatible type for operation. All operands must be booleans!\n");\
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
    vm.code = code;
#ifdef DEBUG
    stringtable_debug();
    symtable_debug();
    bcchunk_disassemble("Current bytecode", vm.code);
    
#endif

    const char* entry = ENTRY_FUNCTION_NAME;
    obj_id_t* ptr = symtable_findstr(entry, strlen(entry), hash_string(entry, strlen(entry)));
    value_t func;
    if(ptr == NULL || !symtable_get(ptr,&func) || !IS_OBJFUNCTION(func))
        user_error_printf("Failed to find '%s' entry function\n", entry);
    if(AS_OBJFUNCTION(func)->entry_offset < 0)
        user_error_printf("Function '%s' is declared but not defined\n", entry);
    if(AS_OBJFUNCTION(func)->base.argc != 0)
        user_error_printf("Function '%s' must not have any arguments\n", entry);
    vm.ip = &vm.code->_code.data[AS_OBJFUNCTION(func)->entry_offset];

    return interpret();
}

static void vm_free(){}

static vm_execute_result interpret(){
    value_t val;
    int is_done = 0;
    while (!is_done) {
        byte_t instruction = read_byte();
#ifdef DEBUG
        printf("Current instruction: %04X | %s\n",(unsigned int)(vm.ip - vm.code->_code.data - 1), op_to_string(instruction));
        printf("BP = 0x%lX SP = 0x%lX\n", vm.bp - vm.stack, vm.sp - vm.stack);
#endif
        switch (instruction) {
            case OP_RETURN:{
                if(vm.bp == &vm.stack[0])
                    is_done = 1;
                else{
                    value_t ret_val = stack_pop();
                    epilogue();
                    value_t ret_ip = stack_pop();
                    vm.ip = &vm.code->_code.data[AS_NUMBER(ret_ip)];
                    stack_push(ret_val);
                }
                break;
            }
            case OP_POP:
                --vm.sp;
#ifdef DEBUG
            if(vm.stack > vm.sp)
                fatal_printf("Stack smashed! Check OP_POP instruction.\n");
            examine_stack();
#endif
                break;
            case OP_POPN:
                vm.sp -= read_constant();
#ifdef DEBUG
            if(vm.stack > vm.sp)
                fatal_printf("Stack smashed! Check OP_POPN instruction.\n");
            examine_stack();
#endif
                break;
            case OP_CLARGS:{
                value_t temp = stack_pop();
                vm.sp -= read_constant();
                stack_push(temp);
                break;
            }
            case OP_NUMBER:
                stack_push(VALUE_NUMBER(extract_value(read_constant()).number));
                break;
            case OP_BOOLEAN:
                stack_push(VALUE_BOOLEAN(extract_value(read_constant()).boolean));
                break;
            case OP_STRING:
                stack_push(VALUE_OBJ(extract_value(read_constant()).obj));
                break;
            case OP_NULL:
                stack_push(VALUE_NULL);
                break;
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
            case OP_GET_LOCAL:{
                int idx = extract_value(read_constant()).number;
                stack_push(vm.bp[idx]);
                break;
            }
            case OP_SET_LOCAL:{
                int idx = extract_value(read_constant()).number;
                value_t val = stack_pop();
                if(!is_value_same_type(val, vm.bp[idx]))
                    interpret_error_printf(get_vm_codeline(), "Incorrect assignment type\n");
                vm.bp[idx] = val;
                stack_push(vm.bp[idx]);
                break;
            }
            case OP_ADD:{
                value_t b = stack_pop();
                value_t a = stack_pop();
                if(IS_NUMBER(a) && IS_NUMBER(b)){
                    stack_push(VALUE_NUMBER(AS_NUMBER(a) + AS_NUMBER(b)));
                }else if(IS_OBJSTRING(a) && IS_OBJSTRING(b)){
                    stack_push(VALUE_OBJ(objstring_conc(a,b)));
                }else{
                    interpret_error_printf(get_vm_codeline(), "Incompatible types for operation.\n");\
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
                }else if(IS_OBJSTRING(a) && IS_OBJSTRING(b)){
                    stack_push(VALUE_BOOLEAN(AS_OBJSTRING(a) == AS_OBJSTRING(b)));
                }else{
                    interpret_error_printf(get_vm_codeline(), "Incompatible types for operation!\n");
                }
                break;
            }
            case OP_GREATER:{
                value_t b = stack_pop();
                value_t a = stack_pop();
                if(IS_NUMBER(a) && IS_NUMBER(b)){
                    stack_push(VALUE_BOOLEAN(AS_NUMBER(a) > AS_NUMBER(b)));
                }else if(IS_BOOLEAN(a) && IS_BOOLEAN(b)){
                    stack_push(VALUE_BOOLEAN(AS_BOOLEAN(a) > AS_BOOLEAN(b)));
                }else if(IS_OBJSTRING(a) && IS_OBJSTRING(b)){
                    stack_push(VALUE_BOOLEAN(strcmp(AS_OBJSTRING(a)->str, AS_OBJSTRING(b)->str) > 0));
                }else{
                    interpret_error_printf(get_vm_codeline(), "Incompatible types for operation!\n");
                }
                break;
            }
            case OP_LESS:{
                value_t b = stack_pop();
                value_t a = stack_pop();
                if(IS_NUMBER(a) && IS_NUMBER(b)){
                    stack_push(VALUE_BOOLEAN(AS_NUMBER(a) < AS_NUMBER(b)));
                }else if(IS_BOOLEAN(a) && IS_BOOLEAN(b)){
                    stack_push(VALUE_BOOLEAN(AS_BOOLEAN(a) < AS_BOOLEAN(b)));
                }else if(IS_OBJSTRING(a) && IS_OBJSTRING(b)){
                    stack_push(VALUE_BOOLEAN(strcmp(AS_OBJSTRING(a)->str, AS_OBJSTRING(b)->str) < 0));
                }else{
                    interpret_error_printf(get_vm_codeline(), "Incompatible types for operation!\n");
                }
                break;
            }
            case OP_JUMP:{
                vm.ip += read_constant();
                break;
            }
            case OP_FJUMP:{
                val = stack_pop();
                int jump = read_constant();
                if(!IS_BOOLEAN(val))
                    interpret_error_printf(get_vm_codeline(), "Expected logical expression\n");
                if(!AS_BOOLEAN(val))
                    vm.ip += jump;
                break;
            }

            #define EXTRACT_GLOBAL(id, val) do{ \
                id = (obj_id_t*)extract_value(read_constant()).obj; \
                if(!symtable_get(id, &val)) \
                    interpret_error_printf(get_vm_codeline(), "Undefined identifier '%s'\n", id->str); \
                if(!IS_NUMBER(val)) \
                    interpret_error_printf(get_vm_codeline(), "Inapropriate value type for increment/decrement\n"); \
            }while(0)
            
            #define PREF_OP_GLOBAL(op) do{\
                obj_id_t* id; \
                value_t val; \
                EXTRACT_GLOBAL(id, val); \
                op AS_NUMBER(val); \
                symtable_set(id, val);\
                stack_push(val);\
            }while(0)

            #define POST_OP_GLOBAL(op) do{\
                obj_id_t* id; \
                value_t val; \
                EXTRACT_GLOBAL(id, val); \
                stack_push(val);\
                op AS_NUMBER(val); \
                symtable_set(id, val);\
            }while(0)

            #define POST_OP_LOCAL(op) do{\
                int idx = extract_value(read_constant()).number;\
                if(!IS_NUMBER(vm.bp[idx]))\
                    interpret_error_printf(get_vm_codeline(), "Inapropriate value type for increment/decrement\n");\
                stack_push(vm.bp[idx]);\
                op AS_NUMBER(vm.bp[idx]);\
            }while(0)

            #define PREF_OP_LOCAL(op) do{\
                int idx = extract_value(read_constant()).number;\
                if(!IS_NUMBER(vm.bp[idx]))\
                    interpret_error_printf(get_vm_codeline(), "Inapropriate value type for increment/decrement\n");\
                op AS_NUMBER(vm.bp[idx]);\
                stack_push(vm.bp[idx]);\
            }while(0)

            case OP_PREFINCR_GLOBAL:{
                PREF_OP_GLOBAL(++);
                break;
            }
            case OP_POSTINCR_GLOBAL:{
                POST_OP_GLOBAL(++);
                break;
            }
            case OP_PREFDECR_GLOBAL:{
                PREF_OP_GLOBAL(--);
                break;
            }
            case OP_POSTDECR_GLOBAL:{
                POST_OP_GLOBAL(--);
                break;
            }
            case OP_POSTINCR_LOCAL:{
                POST_OP_LOCAL(++);
                break;
            }
            case OP_POSTDECR_LOCAL:{
                POST_OP_LOCAL(--);
                break;
            }
            case OP_PREFINCR_LOCAL:{
                PREF_OP_LOCAL(++);
                break;
            }
            case OP_PREFDECR_LOCAL:{
                PREF_OP_LOCAL(--);
                break;
            }

            #undef EXTRACT_GLOBAL
            #undef PREF_OP_GLOBAL
            #undef PREF_OP_LOCAL
            #undef POST_OP_GLOBAL
            #undef POST_OP_LOCAL
            case OP_CALL:{
                obj_function_t* p = (obj_function_t*)extract_value(read_constant()).obj;
                if(p->entry_offset < 0)
                    interpret_error_printf(get_vm_codeline(), "Function '%s' is declared but not defined\n", p->base.name->str);
                stack_push(VALUE_NUMBER(vm.ip - vm.code->_code.data));
                vm.ip = &vm.code->_code.data[p->entry_offset];
                preamble();
                break;
            }
            case OP_NATIVE_CALL:{
                int argc = AS_NUMBER(stack_pop());
                obj_natfunction_t* p = (obj_natfunction_t*)extract_value(read_constant()).obj;
                stack_push(p->impl(argc, vm.sp - argc));
                break;
            }
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
    examine_stack();
#endif
}
static inline value_t stack_pop(){
    if(vm.sp > VM_STACK_START){
#ifdef DEBUG
        --vm.sp;

        examine_stack();
        return *vm.sp;
#else
        return *--vm.sp;
#endif
    }
    fatal_printf("Stack smashed!\n");
}

int get_vm_codeline(){
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
        interpret_error_printf(get_vm_codeline(), "Undefined identifier %s\n", id->str);
    return val;
}

static void set_variable_value(obj_id_t* id, value_t value){
    int idx = resolve_local(id);
    if(idx != -1){
        if(!is_value_same_type(value, vm.bp[idx]))
            interpret_error_printf(get_vm_codeline(), "Incorrect assignment type for '%s'\n", id->str);
        vm.bp[idx] = value;
    }else {
        value_t var_val;
        if(!symtable_get(id, &var_val) || var_val.type == VT_NULL)  
            interpret_error_printf(get_vm_codeline(), "Undefined identifier %s\n", id->str);
        if(!is_value_same_type(var_val, value))
            interpret_error_printf(get_vm_codeline(), "Incorrect assignment type for '%s'\n", id->str);
        symtable_set(id, value);
    }
}

static void preamble(){
    stack_push(VALUE_NUMBER(vm.bp - vm.stack));
    vm.bp = vm.sp;
}

static void epilogue(){
    vm.sp = vm.bp;
    value_t val = stack_pop();
#ifdef DEBUG
    if(!IS_NUMBER(val))
        fatal_printf("Stack smashed! Expected BP offset.\n");
#endif          
    vm.bp = &vm.stack[AS_NUMBER(val)];
}

#ifdef DEBUG
static void examine_stack(){
    printf("\t=== Stack ===\n");
    for(value_t* ptr = vm.stack; ptr < vm.sp; ptr++)
        printf("\t%04X | %s\n", (unsigned)(ptr - vm.stack), examine_value(*ptr));
}
#endif