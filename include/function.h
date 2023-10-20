#ifndef FUNCTION_H
#define FUNCTION_H

#include "defines.h"

typedef double (*functionptr)(double* arguments);

typedef struct func_t {
    functionptr ptr;
    u32 arity;
} Function;

extern const Function NONE;
extern const Function ADD;
extern const Function SUBTRACT;
extern const Function MULTIPLY;
extern const Function DIVIDE;
extern const Function ASSIGN;

#endif /* ! FUNCTION_H */
