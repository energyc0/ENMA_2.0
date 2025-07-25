#ifndef GARBAGE_COLLECTOR_H
#define GARBAGE_COLLECTOR_H

/*
Garbage collector stores pointers to obj_t and frees them periodically
*/

#include "lang_types.h"

#define GARBAGE_COLLECTOR_BASE_SIZE (1024)

//stores new entity
void gc_add(obj_t* obj);

void gc_cleanup();

#endif