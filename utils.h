#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include "scanner.h"

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

void fatal_printf(const char* fmt, ...);
void compile_error_printf(const char* fmt, ...);
void user_error_printf(const char* fmt, ...);

#endif