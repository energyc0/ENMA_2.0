#include "cycler.h"
#include "bytecode.h"
#include "utils.h"
#include <stdlib.h>

typedef enum stat_type{
    CON,
    BRK
}stat_type;


typedef struct stat_info{
    stat_type type;
    int offset;
    struct stat_info* next;
}stat_info;

typedef struct cycle_head{
    int infos;
    int cycle_start;
    struct stat_info* root;
    struct cycle_head* next;
}cycle_head;

struct cycler{
    int cycle_depth;
    struct cycle_head* root;
}cycler;

#define UPDATE_ENDJUMP_LENGTH(chunk, offset) bcchunk_rewrite_constant(chunk, (offset), bcchunk_get_codesize(chunk) - (offset) - sizeof(int))
#define UPDATE_STARTJUMP_LENGTH(chunk, cycle_start,offset, line) bcchunk_write_constant((chunk), (cycle_start) - ((offset) + sizeof(int)), (line));

static stat_info* mk_info(stat_type type, int offset);
static stat_info* mk_continue_info(int offset);
static stat_info* mk_break_info(int offset);

static cycle_head* mk_cycle_head(int start);
//free linklist and update jump lenght
static void free_cycle_head(cycle_head* ptr, struct bytecode_chunk* chunk);

static void add_break_info(cycle_head* ptr, int offset);
static void add_cont_info(cycle_head* ptr, int offset);
static void add_info(cycle_head* ptr, int offset, stat_info*(mk_info)(int));

bool is_cycle(){
    return cycler.cycle_depth > 0;
}

//start counting 'break's
void start_parse_cycle(struct bytecode_chunk* chunk){
    cycle_head* ptr =  mk_cycle_head(bcchunk_get_codesize(chunk));
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

void parse_continue(struct bytecode_chunk* chunk, op_t op, int line){
    bcchunk_write_simple_op(chunk, op, line);
    int offset = bcchunk_get_codesize(chunk);
    bcchunk_write_constant(chunk, -(int)sizeof(int), line);
    add_cont_info(cycler.root, offset);
}

void change_start_offset(int offset){
    cycler.root->cycle_start = offset;
}

//update jump operation constants
void end_parse_cycle(struct bytecode_chunk* chunk){
    cycler.cycle_depth--;
    cycle_head* ptr = cycler.root;
    cycler.root = cycler.root->next;
    free_cycle_head(ptr, chunk);
}

static cycle_head* mk_cycle_head(int start){
    cycle_head* ptr = malloc(sizeof(cycle_head));
    ptr->infos = 0;
    ptr->next = NULL;
    ptr->root = NULL;
    ptr->cycle_start = start;
    return ptr;
}

static void free_cycle_head(cycle_head* ptr, struct bytecode_chunk* chunk){
    for(stat_info* p = ptr->root; p != NULL;){
        stat_info* temp = p->next;
        switch(p->type){
            case BRK: 
                UPDATE_ENDJUMP_LENGTH(chunk, p->offset);
                break;
            case CON:
                bcchunk_rewrite_constant(chunk, p->offset, ptr->cycle_start - (p->offset + sizeof(int)));
                break;
            default:
                fatal_printf("Undefined stat_info type!\n");
        }
        free(p);
        p = temp;
    }
    free(ptr);
}

static void add_info(cycle_head* ptr, int offset, stat_info*(mk_info)(int)){
    ptr->infos++;
    stat_info* p = mk_info(offset);
    p->next = ptr->root;
    ptr->root = p; 
}

static void add_break_info(cycle_head* ptr, int offset){
    add_info(ptr, offset, mk_break_info);
}

static void add_cont_info(cycle_head* ptr, int offset){
    add_info(ptr, offset, mk_continue_info);
}

static stat_info* mk_info(stat_type type, int offset){
    stat_info* ptr = malloc(sizeof(stat_info));
    ptr->offset = offset;
    ptr->next = NULL;
    ptr->type = type;
    return ptr;
}

static stat_info* mk_break_info(int offset){
    return mk_info(BRK, offset);
}

static stat_info* mk_continue_info(int offset){
    return mk_info(CON, offset);
}