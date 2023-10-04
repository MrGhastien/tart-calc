#include "token.h"
#include "darray.h"
#include "util.h"

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

static Token *createNumber(const char *symbol);
static const factory factories[] = {operatorFromSymbol, createNumber};

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

Token *createToken(Identifier identifier, const char *symbol) {
    if (identifier < LPAREN) {
        return factories[identifier](symbol);
    } else if (identifier < _IDENTIFIER_SIZE) {
        return ((Token *)prebuilt.a) + identifier - LPAREN;
    } else {
        return null;
    }
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

// === Factories ===
static Token *createNumber(const char *symbol) {
    double n = strtod(symbol, null);
    Token *t = malloc(sizeof *t);
    t->function = NONE;
    t->identifier = NUMBER;
    t->symbol = symbol;
    t->value.number = n;
    return t;
}
