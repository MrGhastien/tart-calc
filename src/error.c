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
    MSG(ERR_INVALID_EXPR, "Malformed expression.");
    MSG(ERR_NO_INPUT, "No input.");
    MSG(ERR_UNDEFINED_VAR, "Undefined variable.");
}

void signalError(enum errortype type, Token* token) {
    if (!initialized) {
        err(ERR_SYSTEM_UNINIT, "Attempt to report error while the system has not been initialized.");
        return;
    }
    Error error = {type, !token ? 0 : token->position, true, {.token = token}, false};
    darrayAdd(errors, error);
}

void signalErrorNoToken(enum errortype type, char* symbol, u64 position) {
    if (!initialized) {
        err(ERR_SYSTEM_UNINIT, "Attempt to report error while the system has not been initialized.");
        return;
    }
    Error error;
    error.hasToken = false;
    error.position = position;
    error.type = type;
    if (!symbol) {
        error.value.symbol[0] = 0;
    } else {
        u64 symbolLen = strlen(symbol);
        error.symbolTooLong = symbolLen >= ERR_SYMBOL_MAX;
        memcpy(error.value.symbol, symbol, error.symbolTooLong ? ERR_SYMBOL_MAX - 1 : symbolLen);
        // If the string is too long, the null character is not copied.
        // We need to add it manually.
        if (error.symbolTooLong)
            error.value.symbol[ERR_SYMBOL_MAX - 1] = 0;
    }
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

static void previewExprError(const char* expression, const char* symbol, size_t pos, bool tooLongSymbol) {
    size_t symbolLen = strlen(symbol);
    u64 exprlen = strlen(expression);
    fputs("\x1b[22m", stderr);
    u64 minIndex = pos < 50 ? 0 : pos - 50;
    u64 maxIndex = pos + 50;
    if(minIndex > 0)
        fputs("...", stderr);
    for (size_t i = minIndex; expression[i] && i < maxIndex; i++) {
        if (i == pos)
            fputs("\x1b[31;1m", stderr);
        else if (i == pos + symbolLen)
            fputs("\x1b[39;22m", stderr);
        fputc(expression[i], stderr);
    }
    if(exprlen > maxIndex)
        fputs("...", stderr);
    size_t j = 0;
    for (j = 0; j < pos; j++) {
        fputc(' ', stderr);
    }
    fputs("\e[31;1m", stderr);
    fputc('^', stderr);
    for (j = 0; j < symbolLen - 1; j++) {
        fputc('-', stderr);
    }
    if(tooLongSymbol)
        fputs("...", stderr);
    fprintf(stderr, "%s\n", "\e[0m");
}

void printErrors(const char* expression) {
    u64 errCount = getErrorCount();
    for (u64 i = 0; i < errCount; i++) {
        Error* err = ((Error*)errors->a) + i;
        if (err->position == (u64)-1) {
            fprintf(stderr, "Error when evaluating : %s\n", messages[err->type]);
            continue;
        }

        printf("Error at position %zu : %s\n", err->position, messages[err->type]);
        if (err->hasToken) {
            Token* tok = err->value.token;
            fprintf(stderr, "Problematic token : '%s' [%i]\n", tok->symbol, tok->identifier);
            previewExprError(expression, tok->symbol, err->position, false);
        } else {
            fprintf(stderr, "Erroneous symbol : %s", err->value.symbol);
            if(err->symbolTooLong)
                fputs("...", stderr);
            fputc('\n', stderr);
            previewExprError(expression, err->value.symbol, err->position, err->symbolTooLong);
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
