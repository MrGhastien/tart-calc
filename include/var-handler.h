#ifndef VAR_HANDLER_H
#define VAR_HANDLER_H

#include "defines.h"

typedef struct varctx_t VarCtx;

void initVariables();
void shutVariables();

bool setVariable(const char* symbol, double value);
bool getVariable(const char* symbol, double* outValue);

#endif /* ! VAR_HANDLER_H */
