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
    const char* symbol;
    union data {
        double number;
        Operator operator;
    } value;
    Function function;
} Token;

enum OperatorType {
    OPERATOR_ADD,
    OPERATOR_SUBTRACT,
    OPERATOR_MULTIPLY,
    OPERATOR_DIVIDE,
    _OPERATOR_SIZE
};

typedef Token *(*factory)(const char *symbol);

const char* getSymbol(Identifier identifier);
Identifier getIdentifier(const char* symbol);
Token* createToken(Identifier identifier, const char* symbol);
bool isGeneric(Identifier identifier);

Token *getOperator(enum OperatorType type);
Token *operatorFromSymbol(const char *str);
void initOperators();
