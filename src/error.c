#include "error.h"
#include "util.h"

#include <err.h>
#include <stdio.h>

#define MSG(code, msg) messages[code] = msg

static darray* errors;
static u64 errIndex;
static bool initialized = false;

static char* messages[_ERR_SIZE];

void initErrorSystem() {
    if (initialized)
        return;
    errors = darrayCreate(4, sizeof(Error));
    errIndex = 0;
    initialized = true;

    MSG(ERR_OP_MISSING_OPERAND, "Operator is missing one or more operands.");
    MSG(ERR_FUNC_MISSING_OPERAND, "Function is missing one or more operands.");
    MSG(ERR_MISMATCH_PAREN, "Mismatched parentheses.");
    MSG(ERR_ALLOC_FAIL, "Memory allocation failed.");
}

void signalError(enum errortype type, Token* token) {
    if (!initialized) {
        err(ERR_SYSTEM_UNINIT, "Attempt to report error while the system has not been initialized.");
        return;
    }
    Error error = {type, !token ? 0 : token->position, token};
    darrayAdd(errors, error);
}

u64 getErrorCount() {
    if (!initialized) {
        err(ERR_SYSTEM_UNINIT, "Attempt to get error count while the system has not been initialized.");
        return 0;
    }
    return darrayLength(errors);
}

Error* getNextError() {
    if (!initialized) {
        err(ERR_SYSTEM_UNINIT, "Attempt to get error while the system has not been initialized.");
        return null;
    }
    if (errIndex >= darrayLength(errors))
        return null;
    return ((Error*)errors->a) + errIndex++;
}

void printErrors(const char* expression) {
    u64 errCount = getErrorCount();
    for (u64 i = 0; i < errCount; i++) {
        Error* err = ((Error*)errors->a) + i;
        if (err->token) {
            printf("Error at position %zu : %s\n", err->position, messages[err->type]);
            printf("Problematic token : '%s' [%i]\n", err->token->symbol, err->token->identifier);
            printf("\e[1m%s", expression);
            size_t j = 0;
            for (j = 0; j < err->position; j++) {
                putchar(' ');
            }
            putchar('^');
            size_t symbolLen = strlen(err->token->symbol);
            for (j = 0; j < symbolLen - 1; j++) {
                putchar('~');
            }
            printf("%s\n", "\e[0m");
        } else {
            printf("Error : %s\n", messages[err->type]);
        }
    }
    darrayClear(errors);
}

void shutErrorSystem() {
    if (!initialized)
        return;
    darrayDestroy(errors);
    initialized = false;
}
