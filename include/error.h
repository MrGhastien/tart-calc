#ifndef ERROR_H
#define ERROR_H

#include "defines.h"
#include "token.h"

enum errortype {
    ERR_SYSTEM_UNINIT = -1,

    ERR_OP_MISSING_OPERAND,
    ERR_FUNC_MISSING_OPERAND,
    ERR_MISMATCH_PAREN,
    ERR_ALLOC_FAIL,

    _ERR_SIZE
};

typedef struct err {
    enum errortype type;
    u64 position;
    Token* token;
} Error;

void initErrorSystem();
void shutErrorSystem();

void signalError(enum errortype type, Token* token);

u64 getErrorCount();
Error* getNextError();

void printErrors(const char* expression);

#endif /* ! ERROR_H */
