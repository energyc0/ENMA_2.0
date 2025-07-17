#include "vm.h"
#include "bytecode.h"
#include "utils.h"
#include <stdio.h>

static struct virtual_machine vm;

#define VM_STACK_START (vm.stack)
#define VM_STACK_END (VM_STACK_START + sizeof(vm.stack) / sizeof(vm.stack[0]))

static vm_execute_result interpret();
//for debug purposes
static void examine_stack(); 

static inline void stack_push(vm_word_t data);
static inline vm_word_t stack_pop();

//reads byte and increases ip pointer
static inline byte_t read_byte();
//reads 3 bytes and makes 4 byte constant (little-endian)
static inline vm_word_t read_constant();
//pops two values from the stack and pushes the result
#define CALC_OP(op) do{ \
        vm_word_t b = stack_pop(); \
        vm_word_t a = stack_pop(); \
        stack_push(a op b); \
    } while(0)

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
    int is_done = 0;
    while (!is_done) {
        switch (read_byte()) {
            case OP_RETURN: 
                is_done = 1;
                break;
            case OP_CONSTANT:
                stack_push(read_constant());
                break;
            case OP_ADD:
                CALC_OP(+);
                break;
            case OP_SUB: 
                CALC_OP(-);
                break;
            case OP_DIV: 
                CALC_OP(/);
                break;
            case OP_MUL: 
                CALC_OP(*);
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

static inline vm_word_t read_constant(){
    return (read_byte() + (read_byte() << 8)  + (read_byte() << 16));
}

static inline void stack_push(vm_word_t data){
    if(vm.stack_top < VM_STACK_END)
        *vm.stack_top++ = data;
    else
        fatal_printf("Stack overflow!\n");
#ifdef DEBUG
    printf("After push:\n");
    examine_stack();
#endif
}
static inline vm_word_t stack_pop(){
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

static void examine_stack(){
    printf("=== Stack ===\n");
    for(vm_word_t* ptr = vm.stack; ptr < vm.stack_top; ptr++)
        printf("%04X | %d\n", (unsigned)(ptr - vm.stack), *ptr);
}