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

EvalNode* parse(darray* tokens) {
    if (getErrorCount() > 0)
        return null;
    ParsingCtx ctx;
    darrayInit(&ctx.operatorStack, 4, sizeof(Token*));
    darrayInit(&ctx.outputQueue, 4, sizeof(Token*));

    Token* t = null;
    for (u64 i = 0; i < darrayLength(tokens); i++) {
        darrayGet(tokens, i, &t);
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
            return NULL;
        }
        popOperator(&ctx);
    }
    if (getErrorCount() > 0)
        return NULL;
    EvalNode* node;
    darrayGet(&ctx.outputQueue, 0, &node);
    return node;
}

static void destroyToken(void* ptr) {
    Token** tok = ptr;
    free((*tok)->symbol);
    free(*tok);
}

double evaluate(const char* expression) {
    darray tokenBuffer;
    darrayInit(&tokenBuffer, 4, sizeof(Token));
    tokenize(expression, &tokenBuffer);
    EvalNode* tree = parse(&tokenBuffer);
    double result = treeEval(tree);
    for (u64 i = 0; i < darrayLength(&tokenBuffer); i++) {
        Token* t = tokenBuffer.a + i;
        free(t->symbol);
    }
    if (tree)
        treeDestroy(tree);
    if (getErrorCount() > 0)
        printErrors(expression);

    return result;
}
