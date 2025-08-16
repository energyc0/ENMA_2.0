#ifndef GARBAGE_COLLECTOR_H
#define GARBAGE_COLLECTOR_H

/*
Garbage collector stores pointers to obj_t and frees them periodically
*/

#include "lang_types.h"

#define GARBAGE_COLLECTOR_BASE_SIZE (1024)

//stores new entity
void gc_add(obj_t* obj);
void gc_collect();
void gc_cleanup();

#ifdef DEBUG_GC

void gc_debug();

#define GCRUNDEBUG() gc_debug()
#define GCLOG(...) printf(__VA_ARGS__)

#else

#define GCLOG(...) do {} while(0)
#define GCRUNDEBUG() do {} while(0)

#endif

#ifdef DEBUG_STRESS_GC
    #define GCTEST() gc_collect()
#else
    #define GCTEST() do {} while(0)
#endif

#endif