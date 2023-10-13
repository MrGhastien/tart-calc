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

bool setCurrentId(Identifier newID, Identifier* id, LexerCtx* ctx) {
    bool success = true;
    if (newID == *id)
        return true;
    StringBuilder* builder = &ctx->tokenBuilder;
    if (builderLength(builder) > 0) {
        char* str = builderCreateString(builder);
        Token* t;
        if (*id == _IDENTIFIER_SIZE) {
            t = createToken(getIdentifier(str), str, ctx->tokenPos);
        } else {
            t = createToken(*id, str, ctx->tokenPos);
        }
        if (t == null) {
            signalErrorNoToken(ERR_UNKNOWN_TOKEN, str, ctx->tokenPos);
            success = false;
        } else {
            darrayAdd(ctx->tokens, t);
        }
        builderReset(builder);
    }
    ctx->tokenPos = ctx->position;
    *id = newID;
    return success;
}

darray* tokenize(const char* str) {
    LexerCtx ctx;
    ctx.position = 0;
    ctx.tokenPos = 0;
    initBuilder(&ctx.tokenBuilder);
    ctx.tokens = darrayCreate(4, sizeof(Token*));

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
    free(ctx.tokenBuilder.a);
    return ctx.tokens;
}
