#ifndef CYCLE_H
#define CYCLE_H

#include "bytecode.h"

//start counting 'break's
void start_parse_breaks();

bool is_cycle();

//mark jump operation
void parse_break(struct bytecode_chunk* chunk, op_t op, int line);
//update jump operation constants
void end_parse_breaks(struct bytecode_chunk* chunk);

#endif