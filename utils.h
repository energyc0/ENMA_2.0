#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>

#define fatal_printf(...) do{ \
    fprintf(stderr, "FATAL: " __VA_ARGS__); \
    exit(1); \
}while(0)

#endif