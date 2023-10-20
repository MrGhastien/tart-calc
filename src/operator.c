#include "token.h"
#include "util.h"

#define DEF_OP(name, _symbol, _priority, _rightAssociative) Token* op##name = operators + OPERATOR_##name;\
    op##name->identifier = OPERATOR;\
    op##name->symbol = _symbol;\
    op##name->value.operator.priority = _priority;\
    op##name->value.operator.rightAssociative = _rightAssociative;\
    op##name->value.operator.type = OPERATOR_##name;\
    op##name->function = name


static Token operators[_OPERATOR_SIZE];

void initOperators() {
    DEF_OP(ADD, "+", 2, false);
    DEF_OP(SUBTRACT, "-", 2, false);
    DEF_OP(MULTIPLY, "*", 3, false);
    DEF_OP(DIVIDE, "/", 3, false);
    DEF_OP(ASSIGN, "=", 0, true);
}

bool operatorFromSymbol(const char *str, Token* newToken) {
    for (u64 i = 0; i < _OPERATOR_SIZE; i++) {
        if (streq(operators[i].symbol, str)) {
            Token* op = operators + i;
            newToken->function = op->function;
            newToken->value.operator = op->value.operator;
            return true;
        }
    }
    return false;
}
