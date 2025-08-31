#ifndef NATIVE_FUNCTIONS_H
#define NATIVE_FUNCTIONS_H

#include "lang_types.h"

value_t native_clock(int argc, value_t* argv);
value_t native_print(int argc, value_t* argv);
value_t native_println(int argc, value_t* argv);

value_t native_isnum(int argc, value_t* argv);
value_t native_isstr(int argc, value_t* argv);
value_t native_isbool(int argc, value_t* argv);
value_t native_isinst(int argc, value_t* argv);

#endif 