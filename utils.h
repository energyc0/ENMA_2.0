#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

#define fatal_printf(...) do{ \
    eprintf("FATAL: " __VA_ARGS__); \
    exit(1); \
}while(0)

#endif