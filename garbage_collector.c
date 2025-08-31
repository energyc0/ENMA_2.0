#include "garbage_collector.h"
#include "hash_table.h"
#include "lang_types.h"
#include <stdio.h>
#include "utils.h"
#include "vm.h"

static obj_t* root = NULL;
extern struct virtual_machine vm;
extern struct hash_table symtable;

#ifdef DEBUG_GC
    void print_gc_entry(obj_t* entry);
    #define GCPRINTENTRY(entry) print_gc_entry(entry)
#else
    #define GCPRINTENTRY(entry) do {} while(0)
#endif

static void mark_object(obj_t* obj);
static void mark_stack();
static void mark_stack_frame(value_t* bp, int count);
static void mark_table(struct hash_table* t);

void gc_add(obj_t* obj){
    obj->next = root;
    root = obj;
    GCLOG("\nGarbage collector added new entry: ");
    GCPRINTENTRY(obj);
}

void gc_cleanup(){
    GCRUNDEBUG();
    GCLOG("Garbage collector is cleaning all the stuff.\n");
    for (; root != NULL;) {
        obj_t* temp = root->next;
        obj_free(root);
        root = temp;
    }
    GCLOG("Garbage collector has finished cleaning.\n");
}

void gc_collect(){
    if(vm.ip == NULL)
        return;
    mark_stack();
    mark_table(&symtable);
    
    while(root && !root->is_marked){
        obj_t* temp = root;
        root = root->next;
        obj_free(temp);
    }
    if(!root)
        return;
    root->is_marked = false;
    obj_t* prev = root;
    for(obj_t* p = root->next; ; prev = p, p = p->next){
        while(p && !p->is_marked){
            obj_t* temp = p;
            GCLOG("\nGarbage collector deleted entry: ");
            GCPRINTENTRY(temp);
            prev->next = p->next;
            p = p->next;
            obj_free(temp);
        }
        if(!p)
            break;
        p->is_marked = false;
    }
}

static void mark_object(obj_t* obj){
    switch(obj->type){
        case OBJ_STRING: case OBJ_IDENTIFIER:
            break;
        case OBJ_FUNCTION: case OBJ_NATFUNCTION:
            mark_object((obj_t*)((obj_func_base_t*)obj)->name);
            break;
        default:
            fatal_printf("Undefined obj_t type in mark_object()\n");
    }
    obj->is_marked = true;
}

static void mark_stack_frame(value_t* bp, int count){
    for(;count > 0; count--, bp++)
        if(IS_OBJ(*bp))
            mark_object(AS_OBJ(*bp));
}

static void mark_stack(){
    //stack frame consists of [return address, old bp, variables]
    value_t* start = vm.bp;
    value_t* end = vm.sp;
    do{
        mark_stack_frame(start, end - start);
        end = start - 2;
        if(start > vm.stack)
            start = &vm.stack[(int)AS_NUMBER(start[-1])];
        else
            break;
    }while(true);
}

static void mark_table(struct hash_table* t){
    for(size_t i = 0; i < t->capacity; i++){
        if(t->entries[i].key != NULL)
            mark_object((obj_t*)t->entries[i].key);
        if(IS_OBJ(t->entries[i].value))
            mark_object(AS_OBJ(t->entries[i].value));
    }
}

#ifdef DEBUG_GC
void gc_debug(){
    obj_t* p = root;
    printf("Garbage collector debug start.\n");
    while(p){
        print_gc_entry(p);
        p = p->next;
    }
    printf("Garbage collector debug end.\n");
}

void print_gc_entry(obj_t* p){
    static const char* objnames[] = {
        [OBJ_STRING] = "OBJ_STRING",
        [OBJ_IDENTIFIER] = "OBJ_IDENTIFIER",
        [OBJ_FUNCTION] = "OBJ_FUNCTION",
        [OBJ_NATFUNCTION] = "OBJ_NATFUNCTION"
    };
    switch(p->type){
            case OBJ_STRING: case OBJ_IDENTIFIER: case OBJ_FUNCTION: case OBJ_NATFUNCTION:
                break;
            default:
                fatal_printf("Undefined obj_t type in gc_debug()\n"); 
        }
    printf("%s\n", objnames[p->type]);
    examine_value(VALUE_OBJ(p));
    putchar('\n');
}
#endif