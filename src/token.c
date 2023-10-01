#include "token.h"
#include "util.h"

#include <stdlib.h>

//Can't put this inside 'function.c', because it would not be a compile-time constant anymore.
const Function NONE = {null, 0};

static Token * createNumber(const char *symbol);
static const factory factories[] = {operatorFromSymbol, createNumber};
static Token prebuilt[] = {
    {LPAREN, "(", {0}, NONE},
    {RPAREN, ")", {0}, NONE},
    {SEMI, ";", {0}, NONE},
};

const char *getSymbol(Identifier identifier) {
    if(identifier < LPAREN || identifier == _IDENTIFIER_SIZE)
        return null;

    return prebuilt[identifier - LPAREN].symbol;
}

Identifier getIdentifier(const char *symbol) {
    for (u64 i = 0; i < _IDENTIFIER_SIZE - LPAREN; i++) {
        const char* s = prebuilt[i].symbol;
        const char* t = symbol;
        if(streq(s, t)) 
            return prebuilt[i].identifier;
    }
    return _IDENTIFIER_SIZE;
}

Token* createToken(Identifier identifier, const char* symbol) {
    if (identifier < LPAREN) {
        return factories[identifier](symbol);
    } else if (identifier < _IDENTIFIER_SIZE) {
        return prebuilt + identifier - LPAREN;
    } else {
        return null;
    }
}

bool isGeneric(Identifier identifier) {
    return identifier >= LPAREN && identifier < _IDENTIFIER_SIZE;
}



// === Factories ===
static Token * createNumber(const char *symbol) {
    double n = strtod(symbol, null);
    Token* t = malloc(sizeof *t);
    t->function = NONE;
    t->identifier = NUMBER;
    t->symbol = symbol;
    t->value.number = n;
    return t;
}
