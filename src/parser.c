#include "interpreter.h"
#include "darray.h"
#include "error.h"
#include "util.h"

#include "./pinterpreter.h"
#include <stdlib.h>

static bool popOperator(ParsingCtx* ctx) {
    Token* t;
    darrayPop(&ctx->operatorStack, &t);
    EvalNode* node = treeCreate(t);
    for (u64 i = node->arity; i > 0; i--) {
        EvalNode* n;
        if (!darrayRemove(&ctx->outputQueue, darrayLength(&ctx->outputQueue) - i, &n)) {
            // Operator is missing an operand !
            signalError(ERR_OP_MISSING_OPERAND, t);
            treeDestroy(node);
            return false;
        }
        treeAddChild(node, n);
    }
    darrayAdd(&ctx->outputQueue, node);
    return true;
}

static bool handleOperator(Token* token, ParsingCtx* ctx) {
    Operator* op = &token->value.operator;
    Token* t2;
    while (darrayPeek(&ctx->operatorStack, &t2) && t2->identifier == OPERATOR) {
        Operator* o2 = &t2->value.operator;
        if (o2->priority < op->priority || (o2->priority == op->priority && !op->rightAssociative))
            break;
        if (!popOperator(ctx))
            return false;
    }
    darrayAdd(&ctx->operatorStack, token);
    return true;
}

// The parameter 't' is only used for error reporting
static bool handleParen(ParsingCtx* ctx, Token* parenToken) {
    Token* t;
    while (darrayPeek(&ctx->operatorStack, &t) && t->identifier != LPAREN) {
        if (!popOperator(ctx)) {
            signalError(ERR_MISMATCH_PAREN, t);
            return false;
        }
    }
    if (darrayLength(&ctx->operatorStack) == 0) {
        signalError(ERR_MISMATCH_PAREN, parenToken);
        return false;
    }
    darrayPop(&ctx->operatorStack, null);
    return true;
}

static void freeTree(void* ptr) {
    treeDestroy(*(EvalNode**)ptr);
}

EvalNode* parse(darray* tokens) {
    if (getErrorCount() > 0)
        return null;
    ParsingCtx ctx;
    darrayInit(&ctx.operatorStack, 4, sizeof(Token*));
    darrayInit(&ctx.outputQueue, 4, sizeof(Token*));

    Token* t;
    for (u64 i = 0; i < darrayLength(tokens); i++) {
        t = darrayGetPtr(tokens, i);
        Identifier id = t->identifier;
        EvalNode* node;
        switch (id) {
        case NUMBER:
            node = treeCreate(t);
            darrayAdd(&ctx.outputQueue, node);
            break;
        case OPERATOR:
            handleOperator(t, &ctx);
            break;
        case LPAREN:
            darrayAdd(&ctx.operatorStack, t);
            break;
        case RPAREN:
            handleParen(&ctx, t);
            break;
        default:
            exit(3);
            break;
        }
    }
    Token* op;
    while (darrayPeek(&ctx.operatorStack, &op)) {
        if (op->identifier == LPAREN) {
            signalError(ERR_MISMATCH_PAREN, op);
            break;
        }
        popOperator(&ctx);
    }
    EvalNode* node;
    if (darrayLength(&ctx.outputQueue) > 1) {
        darrayGet(&ctx.outputQueue, 1, &node);
        signalError(ERR_INVALID_EXPR, node->token);
    }
    darrayGet(&ctx.outputQueue, 0, &node);
    if (getErrorCount() > 0) {
        //In this case, destroy all tree nodes we created
        //or else MEMORY LEAKS 
        darrayClearDeep(&ctx.outputQueue, &freeTree);
        darrayClearDeep(&ctx.operatorStack, &freeTree);
        node = null;
    }
    darrayEmpty(&ctx.operatorStack);
    darrayEmpty(&ctx.outputQueue);
    return node;
}

bool evaluate(const char* expression, double* outResult) {
    bool success = true;
    darray tokenBuffer;
    darrayInit(&tokenBuffer, 4, sizeof(Token));
    tokenize(expression, &tokenBuffer);
    EvalNode* tree = parse(&tokenBuffer);
    double result = treeEval(tree);
    *outResult = result;
    if (tree)
        treeDestroy(tree);
    if (getErrorCount() > 0) {
        printErrors(expression);
        success = false;
    }
    for (u64 i = 0; i < darrayLength(&tokenBuffer); i++) {
        Token* t = darrayGetPtr(&tokenBuffer, i);
        free(t->symbol);
    }
    darrayEmpty(&tokenBuffer);
    return success;
}
