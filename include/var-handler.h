#ifndef VAR_HANDLER_H
#define VAR_HANDLER_H

#include "defines.h"
#include "value.h"

typedef struct varctx_t VarCtx;

void initVariables(void);
void shutVariables(void);

bool setVariable(const char* symbol, Value* value);
bool getVariable(const char* symbol, Value* outValue);

#endif /* ! VAR_HANDLER_H */
