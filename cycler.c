#include "cycler.h"
#include "bytecode.h"
#include <stdlib.h>


typedef struct break_info{
    int offset;
    struct break_info* next;
}break_info;

typedef struct cycle_head{
    int breaks;
    struct break_info* root;
    struct cycle_head* next;
}cycle_head;

struct cycler{
    int cycle_depth;
    struct cycle_head* root;
}cycler;

#define UPDATE_JUMP_LENGTH(chunk, offset) bcchunk_rewrite_constant(chunk, (offset), bcchunk_get_codesize(chunk) - (offset) - sizeof(int))

static break_info* mk_break_info(int offset);
static cycle_head* mk_cycle_head();
//free linklist and update jump lenght
static void free_cycle_head(cycle_head* ptr, struct bytecode_chunk* chunk);
static void add_break_info(cycle_head* ptr, int offset);

bool is_cycle(){
    return cycler.cycle_depth > 0;
}

//start counting 'break's
void start_parse_breaks(){
    cycle_head* ptr =  mk_cycle_head();
    ptr->next = cycler.root;
    cycler.root = ptr;
    cycler.cycle_depth++;
}
//mark jump operation
void parse_break(struct bytecode_chunk* chunk, op_t op, int line){
    bcchunk_write_simple_op(chunk, op, line);
    int offset = bcchunk_get_codesize(chunk);
    bcchunk_write_constant(chunk, -(int)sizeof(int), line);
    add_break_info(cycler.root, offset);
}
//update jump operation constants
void end_parse_breaks(struct bytecode_chunk* chunk){
    cycler.cycle_depth--;
    cycle_head* ptr = cycler.root;
    cycler.root = cycler.root->next;
    free_cycle_head(ptr, chunk);
}

static cycle_head* mk_cycle_head(){
    cycle_head* ptr = malloc(sizeof(cycle_head));
    ptr->breaks = 0;
    ptr->next = NULL;
    ptr->root = NULL;
    return ptr;
}

static void free_cycle_head(cycle_head* ptr, struct bytecode_chunk* chunk){
    for(break_info* p = ptr->root; p != NULL;){
        break_info* temp = p->next;
        UPDATE_JUMP_LENGTH(chunk, p->offset);
        free(p);
        p = temp;
    }
    free(ptr);
}

static void add_break_info(cycle_head* ptr, int offset){
    ptr->breaks++;
    break_info* p = mk_break_info(offset);
    p->next = ptr->root;
    ptr->root = p;
}

static break_info* mk_break_info(int offset){
    break_info* ptr = malloc(sizeof(break_info));
    ptr->offset = offset;
    ptr->next = NULL;
    return ptr;
}