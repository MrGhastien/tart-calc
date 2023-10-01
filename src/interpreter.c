#include "darray.h"
#include "util.h"
#include "string-builder.h"
#include "interpreter.h"

#include "./pinterpreter.h"
#include <stdlib.h>
#include <err.h>

static bool isWhitespace(char c) {
    return c == ' ';
}

static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

Token** tokenize(const char *str) {
    StringBuilder* tokenBuilder = createBuilder();
    Token** tokens = darrayCreate(4, sizeof *tokens);

    char c;
    u64 len = strlen(str);
    Identifier currentId = _IDENTIFIER_SIZE; //Current token type / identifier.
    for (u64 i = 0; i < len; i++) {
        c = str[i];
        if(c == '\n')
            break;

        if (isWhitespace(c)) {
            setCurrentId(_IDENTIFIER_SIZE, &currentId, tokenBuilder, tokens);
            continue;
        }

        if (isDigit(c) || c == '.') {
            if(currentId != NUMBER)
                setCurrentId(NUMBER, &currentId, tokenBuilder, tokens);
            builderAppendc(tokenBuilder, c);
            continue;
        }

        if(isGeneric(getIdentifier(builderStringRef(tokenBuilder)))) {
            //builderDeleteAt(tokenBuilder, builderLength(tokenBuilder) - 1);
            if(currentId != _IDENTIFIER_SIZE)
                setCurrentId(_IDENTIFIER_SIZE, &currentId, tokenBuilder, tokens);
            continue;
        }

        if(currentId != OPERATOR)
            setCurrentId(OPERATOR, &currentId, tokenBuilder, tokens);
        builderAppendc(tokenBuilder, c);
    }
    setCurrentId(_IDENTIFIER_SIZE, &currentId, tokenBuilder, tokens);
    return tokens;
}

static void popOperator(Token **operatorStack, EvalNode **outputQueue) {
    Token* t;
    darrayPop(operatorStack, &t);
    EvalNode* node = treeCreate(t);
    for (u64 i = node->arity; i > 0; i--) {
        EvalNode* n;
        darrayRemove(outputQueue, darrayLength(outputQueue) - i, &n);
        treeAddChild(node, n);
    }
    darrayAdd(outputQueue, &node);
}

static void handleOperator(Token *token, Token **operatorStack, EvalNode **outputQueue) {
    Operator* op = &token->value.operator;
    Token* t2;
    while (darrayPeek(operatorStack, &t2) && t2->identifier == OPERATOR) {
        Operator* o2 = &t2->value.operator;
        if(o2->priority < op->priority || (o2->priority == op->priority && !op->rightAssociative))
            break;
        popOperator(operatorStack, outputQueue);
    }
    darrayAdd(operatorStack, &token);
}

static void handleParen(Token **operatorStack, EvalNode **outputQueue) {
    Token* t;
    while (darrayPeek(operatorStack, &t) && t->identifier != LPAREN) {
        popOperator(operatorStack, outputQueue);
    }
    if(darrayLength(operatorStack) == 0)
        exit(2);
    darrayPop(operatorStack, null);
}

EvalNode *parse(Token **tokens) {
    Token** operatorStack = darrayCreate(4, sizeof *operatorStack);
    EvalNode** outputQueue = darrayCreate(4, sizeof *outputQueue);

    Token* t = null;
    for (u64 i = 0; i < darrayLength(tokens); i++) {
        t = tokens[i];
        Identifier id = t->identifier;
        EvalNode* node;
        switch (id) {
        case NUMBER:
            node = treeCreate(t);
            darrayAdd(outputQueue, &node);
            break;
        case OPERATOR:
            handleOperator(t, operatorStack, outputQueue);
            break;
        case LPAREN:
            darrayInsert(operatorStack, &t, 0);
            break;
        case RPAREN:
            handleParen(operatorStack, outputQueue);
            break;
        default:
            exit(3);
            break;
        }
    }
    Token* op;
    while (darrayPeek(operatorStack, &op)) {
        if (op->identifier == LPAREN) {
            err(2, "Mismatched parentheses\n");
            return NULL;
        }
        popOperator(operatorStack, outputQueue);
    }
    return outputQueue[0];
}

void setCurrentId(Identifier newID, Identifier *id, StringBuilder *tokenBuilder, Token** tokens) {
    if (builderLength(tokenBuilder) > 0) {
        const char* str = builderCreateString(tokenBuilder);
        Token* t;
        if (*id == _IDENTIFIER_SIZE) {
            t = createToken(getIdentifier(str), str);
        } else {
            t = createToken(*id, str);
        }
        if(t == null)
            err(1, "Unknown token '%s'. Current identifier is : '%u'", str, *id);
        darrayAdd(tokens, &t);
        builderReset(tokenBuilder);
    }
    *id = newID;
}
