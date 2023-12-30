#pragma once
#include "darray.h"
#include "function.h"

typedef enum {
    OPERATOR = 0,
    NUMBER,
    ALPHA,

    LPAREN, RPAREN, SEMI,

    _IDENTIFIER_SIZE
} Identifier;

enum OperatorType {
    OPERATOR_ADD,
    OPERATOR_SUBTRACT,
    OPERATOR_MULTIPLY,
    OPERATOR_DIVIDE,
    OPERATOR_MODULO,
    OPERATOR_POWER,
    OPERATOR_ASSIGN,
    _OPERATOR_SIZE
};

typedef struct {
    i8 priority;
    bool rightAssociative;
    enum OperatorType type;
} Operator;

typedef struct {
    Identifier identifier;
    char* symbol;
    union data {
        Value number;
        Operator operator;
    } value;
    Function function;

    //Metadata for error reporting
    u64 position;
} Token;

const char* getSymbol(Identifier identifier);
Identifier getIdentifier(const char* symbol);
bool initToken(Token* tok, Identifier identifier, char* symbol, u64 position);
bool isGeneric(Identifier identifier);

bool operatorFromSymbol(const char *str, Token* newToken);
void initOperators();

void shutTokens();

void initTokens();
