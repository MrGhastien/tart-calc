#pragma once
#include "darray.h"
#include "function.h"

typedef enum {
    OPERATOR = 0,
    NUMBER,

    LPAREN, RPAREN, SEMI,

    _IDENTIFIER_SIZE
} Identifier;

typedef struct {
    i8 priority;
    bool rightAssociative;
} Operator;

typedef struct {
    Identifier identifier;
    char* symbol;
    union data {
        double number;
        Operator operator;
    } value;
    Function function;

    //Metadata for error reporting
    u64 position;
} Token;

enum OperatorType {
    OPERATOR_ADD,
    OPERATOR_SUBTRACT,
    OPERATOR_MULTIPLY,
    OPERATOR_DIVIDE,
    _OPERATOR_SIZE
};

typedef Token *(*factory)(const char *symbol, u64 position);

const char* getSymbol(Identifier identifier);
Identifier getIdentifier(const char* symbol);
Token* createToken(Identifier identifier, const char* symbol, u64 position);
bool isGeneric(Identifier identifier);

bool operatorFromSymbol(const char *str, Token* newToken);
void initOperators();

void shutTokens();

void initTokens();
