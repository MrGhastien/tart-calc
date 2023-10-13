#include "error.h"
#include "util.h"

#include <err.h>
#include <stdio.h>
#include <stdlib.h>

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
    MSG(ERR_UNKNOWN_TOKEN, "Unknown token.");
    MSG(ERR_DIV_BY_ZERO, "Division by zero.");
}

void signalError(enum errortype type, Token* token) {
    if (!initialized) {
        err(ERR_SYSTEM_UNINIT, "Attempt to report error while the system has not been initialized.");
        return;
    }
    Error error = {type, !token ? 0 : token->position, true, {.token = token}};
    darrayAdd(errors, error);
}

void signalErrorNoToken(enum errortype type, char* symbol, u64 position) {
    if (!initialized) {
        err(ERR_SYSTEM_UNINIT, "Attempt to report error while the system has not been initialized.");
        return;
    }
    Error error = {type, position, false, {.symbol = symbol}};
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

static void previewExprError(const char* expression, const char* symbol, size_t pos) {
    size_t symbolLen = strlen(symbol);
    fputs("\x1b[22m", stdout);
    for (size_t i = 0; expression[i]; i++) {
        if (i == pos)
            fputs("\x1b[31;1m", stdout);
        else if (i == pos + symbolLen)
            fputs("\x1b[39;22m", stdout);
        putchar(expression[i]);
    }
    size_t j = 0;
    for (j = 0; j < pos; j++) {
        putchar(' ');
    }
    fputs("\e[31;1m", stdout);
    putchar('^');
    for (j = 0; j < symbolLen - 1; j++) {
        putchar('~');
    }
    printf("%s\n", "\e[0m");
}

void printErrors(const char* expression) {
    u64 errCount = getErrorCount();
    for (u64 i = 0; i < errCount; i++) {
        Error* err = ((Error*)errors->a) + i;
        if (err->position == (u64)-1) {
            printf("Error when evaluating : %s\n", messages[err->type]);
            continue;
        }

        printf("Error at position %zu : %s\n", err->position, messages[err->type]);
        if (err->hasToken) {
            Token* tok = err->value.token;
            printf("Problematic token : '%s' [%i]\n", tok->symbol, tok->identifier);
            previewExprError(expression, tok->symbol, err->position);
        } else {
            printf("Erroneous symbol : %s\n", err->value.symbol);
            previewExprError(expression, err->value.symbol, err->position);
            if (err->value.symbol)
                free(err->value.symbol);
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
