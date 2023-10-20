#include "./pinterpreter.h"
#include "error.h"
#include "interpreter.h"
#include "util.h"

#include <stdlib.h>

static bool isWhitespace(char c) {
    return c == ' ';
}

static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

static bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool endToken(Identifier id, LexerCtx* ctx) {

    if(builderLength(&ctx->tokenBuilder) == 0)
        return true;
    StringBuilder* builder = &ctx->tokenBuilder;
    char* str = builderCreateString(builder);
    Token t;
    bool ok = initToken(&t, id, str, ctx->tokenPos);
    if (!ok) {
        signalErrorNoToken(ERR_UNKNOWN_TOKEN, str, ctx->tokenPos);
        free(str);
        return false;
    } else {
        darrayAdd(ctx->tokens, t);
    }
    builderReset(builder);
    return true;
}

bool setCurrentId(Identifier newID, Identifier* id, LexerCtx* ctx) {
    bool success = true;
    if (newID == *id)
        return true;
    endToken(*id, ctx);
    ctx->tokenPos = ctx->position;
    *id = newID;
    return success;
}

bool tokenize(const char* str, darray* tokenBuffer) {
    LexerCtx ctx;
    ctx.position = 0;
    ctx.tokenPos = 0;
    initBuilder(&ctx.tokenBuilder);
    ctx.tokens = tokenBuffer;

    char c;
    u64 len = strlen(str);
    // The currentId keeps track of the type of the current token we are building
    // We build a token by adding its characters, and then creating the token
    // when we change token types
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
            endToken(currentId, &ctx);
            continue;
        }

        if (c == ')') {
            setCurrentId(RPAREN, &currentId, &ctx);
            builderAppendc(&ctx.tokenBuilder, c);
            endToken(currentId, &ctx);
            continue;
        }

        if (isDigit(c) || c == '.') {
            setCurrentId(NUMBER, &currentId, &ctx);
            builderAppendc(&ctx.tokenBuilder, c);
            continue;
        }

        if (isAlpha(c)) {
            setCurrentId(ALPHA, &currentId, &ctx);
            builderAppendc(&ctx.tokenBuilder, c);
            continue;
        }

        setCurrentId(OPERATOR, &currentId, &ctx);
        builderAppendc(&ctx.tokenBuilder, c);
    }
    endToken(currentId, &ctx);
    free(ctx.tokenBuilder.a);
    return getErrorCount() == 0;
}
