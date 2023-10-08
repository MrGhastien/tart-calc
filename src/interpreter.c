#include "interpreter.h"
#include "darray.h"
#include "string-builder.h"
#include "util.h"
#include "error.h"

#include "./pinterpreter.h"
#include <err.h>
#include <stdio.h>
#include <stdlib.h>

static bool isWhitespace(char c) { return c == ' '; }

static bool isDigit(char c) { return c >= '0' && c <= '9'; }

darray *tokenize(const char *str) {
    LexerCtx ctx;
    ctx.position = 0;
    ctx.tokenPos = 0;
    initBuilder(&ctx.tokenBuilder);
    ctx.tokens = darrayCreate(4, sizeof(Token*));

    char c;
    u64 len = strlen(str);
    //The currentId keeps track of the type of the current token we are building
    //We build a token by adding its characters, and then creating the token
    //when we change token types
    Identifier currentId = _IDENTIFIER_SIZE;
    for (ctx.position = 0; ctx.position < len; ctx.position++) {
        c = str[ctx.position];
        if (c == '\n')
            break;

        if (isWhitespace(c)) {
            setCurrentId(_IDENTIFIER_SIZE, &currentId, &ctx);
            continue;
        }

        if (c == '(') {
            setCurrentId(LPAREN, &currentId, &ctx);
            builderAppendc(&ctx.tokenBuilder, c);
            continue;
        }

        if (c == ')') {
                setCurrentId(RPAREN, &currentId, &ctx);
            builderAppendc(&ctx.tokenBuilder, c);
            continue;
        }

        if (isDigit(c) || c == '.') {
                setCurrentId(NUMBER, &currentId, &ctx);
            builderAppendc(&ctx.tokenBuilder, c);
            continue;
        }

        if (isGeneric(getIdentifier(builderStringRef(&ctx.tokenBuilder)))) {
            // builderDeleteAt(tokenBuilder, builderLength(tokenBuilder) - 1);
                setCurrentId(_IDENTIFIER_SIZE, &currentId, &ctx);
            continue;
        }

        setCurrentId(OPERATOR, &currentId, &ctx);
        builderAppendc(&ctx.tokenBuilder, c);
    }
    setCurrentId(_IDENTIFIER_SIZE, &currentId, &ctx);
    return ctx.tokens;
}

static bool popOperator(ParsingCtx* ctx) {
    Token *t;
    darrayPop(ctx->operatorStack, &t);
    EvalNode *node = treeCreate(t);
    for (u64 i = node->arity; i > 0; i--) {
        EvalNode *n;
        if (!darrayRemove(ctx->outputQueue, darrayLength(ctx->outputQueue) - i, &n)) {
            //Operator is missing an operand !
            signalError(ERR_OP_MISSING_OPERAND, t);
            return false;
        }
        treeAddChild(node, n);
    }
    darrayAdd(ctx->outputQueue, node);
    return true;
}

static bool handleOperator(Token *token, ParsingCtx* ctx) {
    Operator *op = &token->value.operator;
    Token *t2;
    while (darrayPeek(ctx->operatorStack, &t2) && t2->identifier == OPERATOR) {
        Operator *o2 = &t2->value.operator;
        if (o2->priority < op->priority || (o2->priority == op->priority && !op->rightAssociative))
            break;
        if(!popOperator(ctx))
            return false;
    }
    darrayAdd(ctx->operatorStack, token);
    return true;
}

static bool handleParen(ParsingCtx* ctx) {
    Token *t;
    while (darrayPeek(ctx->operatorStack, &t) && t->identifier != LPAREN) {
        if (!popOperator(ctx)) {
            signalError(ERR_MISMATCH_PAREN, t);
            return false;
        }
    }
    if (darrayLength(ctx->operatorStack) == 0)
        return false;
    darrayPop(ctx->operatorStack, null);
    return true;
}

EvalNode *parse(darray *tokens) {
    ParsingCtx ctx;
    ctx.operatorStack = darrayCreate(4, sizeof(Token *));
    ctx.outputQueue = darrayCreate(4, sizeof(EvalNode *));

    Token *t = null;
    for (u64 i = 0; i < darrayLength(tokens); i++) {
        darrayGet(tokens, i, &t);
        Identifier id = t->identifier;
        EvalNode *node;
        switch (id) {
        case NUMBER:
            node = treeCreate(t);
            darrayAdd(ctx.outputQueue, node);
            break;
        case OPERATOR:
            handleOperator(t, &ctx);
            break;
        case LPAREN:
            darrayInsert(ctx.operatorStack, t, 0);
            break;
        case RPAREN:
            if(!handleParen(&ctx))
                return NULL;
            break;
        default:
            exit(3);
            break;
        }
    }
    Token *op;
    while (darrayPeek(ctx.operatorStack, &op)) {
        if (op->identifier == LPAREN) {
            signalError(ERR_MISMATCH_PAREN, op);
            return NULL;
        }
        popOperator(&ctx);
    }
    EvalNode *node;
    darrayGet(ctx.outputQueue, 0, &node);
    return node;
}

void setCurrentId(Identifier newID, Identifier *id, LexerCtx* ctx) {
    if(newID == *id)
        return;
    StringBuilder* builder = &ctx->tokenBuilder;
    if (builderLength(builder) > 0) {
        const char *str = builderCreateString(builder);
        Token *t;
        if (*id == _IDENTIFIER_SIZE) {
            t = createToken(getIdentifier(str), str, ctx->tokenPos);
        } else {
            t = createToken(*id, str, ctx->tokenPos);
        }
        if (t == null)
            err(1, "Unknown token '%s'. Current identifier is : '%u'", str, *id);
        darrayAdd(ctx->tokens, t);
        builderReset(builder);
    }
    ctx->tokenPos = ctx->position;
    *id = newID;
}

double evaluate(const char *expression) {
    darray* tokens = tokenize(expression);
    if (getErrorCount()) {
        return 0;
    }
    EvalNode* tree = parse(tokens);
    if (getErrorCount()) {
        return 0;
    }
    double result = treeEval(tree);

    return result;
}
