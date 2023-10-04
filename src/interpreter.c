#include "interpreter.h"
#include "darray.h"
#include "string-builder.h"
#include "util.h"

#include "./pinterpreter.h"
#include <err.h>
#include <stdlib.h>

static bool isWhitespace(char c) { return c == ' '; }

static bool isDigit(char c) { return c >= '0' && c <= '9'; }

darray *tokenize(const char *str) {
    StringBuilder *tokenBuilder = createBuilder();
    darray *tokens = darrayCreate(4, sizeof(Token *));

    char c;
    u64 len = strlen(str);
    Identifier currentId = _IDENTIFIER_SIZE; // Current token type / identifier.
    for (u64 i = 0; i < len; i++) {
        c = str[i];
        if (c == '\n')
            break;

        if (isWhitespace(c)) {
            setCurrentId(_IDENTIFIER_SIZE, &currentId, tokenBuilder, tokens);
            continue;
        }

        if (c == '(') {
            if (currentId != LPAREN)
                setCurrentId(LPAREN, &currentId, tokenBuilder, tokens);
            builderAppendc(tokenBuilder, c);
            continue;
        }

        if (c == ')') {
            if (currentId != RPAREN)
                setCurrentId(RPAREN, &currentId, tokenBuilder, tokens);
            builderAppendc(tokenBuilder, c);
            continue;
        }

        if (isDigit(c) || c == '.') {
            if (currentId != NUMBER)
                setCurrentId(NUMBER, &currentId, tokenBuilder, tokens);
            builderAppendc(tokenBuilder, c);
            continue;
        }

        if (isGeneric(getIdentifier(builderStringRef(tokenBuilder)))) {
            // builderDeleteAt(tokenBuilder, builderLength(tokenBuilder) - 1);
            if (currentId != _IDENTIFIER_SIZE)
                setCurrentId(_IDENTIFIER_SIZE, &currentId, tokenBuilder, tokens);
            continue;
        }

        if (currentId != OPERATOR)
            setCurrentId(OPERATOR, &currentId, tokenBuilder, tokens);
        builderAppendc(tokenBuilder, c);
    }
    setCurrentId(_IDENTIFIER_SIZE, &currentId, tokenBuilder, tokens);
    return tokens;
}

static void popOperator(darray *operatorStack, darray *outputQueue) {
    Token *t;
    darrayPop(operatorStack, &t);
    EvalNode *node = treeCreate(t);
    for (u64 i = node->arity; i > 0; i--) {
        EvalNode *n;
        darrayRemove(outputQueue, darrayLength(outputQueue) - i, &n);
        treeAddChild(node, n);
    }
    darrayAdd(outputQueue, node);
}

static void handleOperator(Token *token, darray *operatorStack, darray *outputQueue) {
    Operator *op = &token->value.operator;
    Token *t2;
    while (darrayPeek(operatorStack, &t2) && t2->identifier == OPERATOR) {
        Operator *o2 = &t2->value.operator;
        if (o2->priority < op->priority || (o2->priority == op->priority && !op->rightAssociative))
            break;
        popOperator(operatorStack, outputQueue);
    }
    darrayAdd(operatorStack, token);
}

static bool handleParen(darray *operatorStack, darray *outputQueue) {
    Token *t;
    while (darrayPeek(operatorStack, &t) && t->identifier != LPAREN) {
        popOperator(operatorStack, outputQueue);
    }
    if (darrayLength(operatorStack) == 0)
        return false;
    darrayPop(operatorStack, null);
    return true;
}

EvalNode *parse(darray *tokens) {
    darray *operatorStack = darrayCreate(4, sizeof(Token *));
    darray *outputQueue = darrayCreate(4, sizeof(EvalNode *));

    Token *t = null;
    for (u64 i = 0; i < darrayLength(tokens); i++) {
        darrayGet(tokens, i, &t);
        Identifier id = t->identifier;
        EvalNode *node;
        switch (id) {
        case NUMBER:
            node = treeCreate(t);
            darrayAdd(outputQueue, node);
            break;
        case OPERATOR:
            handleOperator(t, operatorStack, outputQueue);
            break;
        case LPAREN:
            darrayInsert(operatorStack, t, 0);
            break;
        case RPAREN:
            if(!handleParen(operatorStack, outputQueue))
                return NULL;
            break;
        default:
            exit(3);
            break;
        }
    }
    Token *op;
    while (darrayPeek(operatorStack, &op)) {
        if (op->identifier == LPAREN) {
            err(2, "Mismatched parentheses\n");
            return NULL;
        }
        popOperator(operatorStack, outputQueue);
    }
    EvalNode *node;
    darrayGet(outputQueue, 0, &node);
    return node;
}

void setCurrentId(Identifier newID, Identifier *id, StringBuilder *tokenBuilder, darray *tokens) {
    if (builderLength(tokenBuilder) > 0) {
        const char *str = builderCreateString(tokenBuilder);
        Token *t;
        if (*id == _IDENTIFIER_SIZE) {
            t = createToken(getIdentifier(str), str);
        } else {
            t = createToken(*id, str);
        }
        if (t == null)
            err(1, "Unknown token '%s'. Current identifier is : '%u'", str, *id);
        darrayAdd(tokens, t);
        builderReset(tokenBuilder);
    }
    *id = newID;
}

double evaluate(const char *expression) {
    darray* tokens = tokenize(expression);
    EvalNode* tree = parse(tokens);
    double result = treeEval(tree);
    return result;
}
