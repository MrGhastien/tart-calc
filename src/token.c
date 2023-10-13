#include "token.h"
#include "darray.h"
#include "util.h"

#include "error.h"

#include <stdlib.h>

// for use inside 'initTokens'.
#define DEF_TOKEN(id, _symbol, _value, _function)                                                                      \
    t.identifier = id;                                                                                                 \
    t.symbol = _symbol;                                                                                                \
    t.value._value;                                                                                                  \
    t.function = _function;                                                                                            \
    _darrayAdd(&prebuilt, &t)

// Can't put this inside 'function.c', because it would not be a compile-time constant anymore.
const Function NONE = {null, 0};

static darray prebuilt;

const char *getSymbol(Identifier identifier) {
    if (identifier < LPAREN || identifier == _IDENTIFIER_SIZE)
        return null;

    Token t;
    darrayGet(&prebuilt, identifier - LPAREN, &t);
    return t.symbol;
}

Identifier getIdentifier(const char *symbol) {
    for (u64 i = 0; i < _IDENTIFIER_SIZE - LPAREN; i++) {
        Token token;
        darrayGet(&prebuilt, i, &token);
        const char *s = token.symbol;
        const char *t = symbol;
        if (streq(s, t))
            return token.identifier;
    }
    return _IDENTIFIER_SIZE;
}

Token *createToken(Identifier identifier, char *symbol, u64 position) {
    Token *t = malloc(sizeof *t);
    if (!t) {
        signalError(ERR_ALLOC_FAIL, null);
        return null;
    }
    t->identifier = identifier;
    t->symbol = symbol;
    t->position = position;
    switch (identifier) {
    case _IDENTIFIER_SIZE:
        free(t);
        return null;
    case OPERATOR:
        if (!operatorFromSymbol(symbol, t)) {
            free(t);
            return NULL;
        }
        break;
    case NUMBER:
        t->value.number = strtod(symbol, null);
        break;
    default:
        t->function = NONE;
        t->value.number = 0;
        break;
    }
    return t;
}

bool isGeneric(Identifier identifier) { return identifier >= LPAREN && identifier < _IDENTIFIER_SIZE; }

void initTokens() {
    darrayInit(&prebuilt, 4, sizeof(Token));
    Token t;
    DEF_TOKEN(LPAREN, "(", number = 0, NONE);
    DEF_TOKEN(RPAREN, ")", number = 0, NONE);
    DEF_TOKEN(SEMI, ";", number = 0, NONE);
}

void shutTokens() { free(prebuilt.a); }
