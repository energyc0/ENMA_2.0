#ifndef CYCLE_H
#define CYCLE_H

#include "bytecode.h"

//start counting 'break's
void start_parse_cycle(struct bytecode_chunk* chunk);

bool is_cycle();

//mark jump operation
void parse_break(struct bytecode_chunk* chunk, op_t op, int line);
//jump to the start of a loop
void parse_continue(struct bytecode_chunk* chunk, op_t op, int line);
//change where to jump after 'continue'
void change_start_offset(int offset);

//update jump operation constants
void end_parse_cycle(struct bytecode_chunk* chunk);

#endif