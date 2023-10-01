#include "token.h"
#include "util.h"

#define DEF_OP(name, _symbol, _priority, _rightAssociative) Token* op##name = operators + OPERATOR_##name;\
    op##name->identifier = OPERATOR;\
    op##name->symbol = _symbol;\
    op##name->value.operator.priority = _priority;\
    op##name->value.operator.rightAssociative = _rightAssociative;\
    op##name->function = name
    

static Token operators[_OPERATOR_SIZE];

void initOperators() {
    DEF_OP(ADD, "+", 2, false);
    DEF_OP(SUBTRACT, "-", 2, false);
    DEF_OP(MULTIPLY, "*", 3, false);
    DEF_OP(DIVIDE, "/", 3, false);
}

Token *getOperator(enum OperatorType type) {
    if (type == _OPERATOR_SIZE)
        return null;
    return operators + type;
}

Token *operatorFromSymbol(const char *str) {
    for (u64 i = 0; i < _OPERATOR_SIZE; i++) {
        if(streq(operators[i].symbol, str))
            return operators + i;
    }
    return null;
}
