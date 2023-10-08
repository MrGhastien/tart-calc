#include "token.h"
#include "string-builder.h"

typedef struct LexerCtx {
    darray* tokens; //Token**
    u64 position;
    u64 tokenPos;
    StringBuilder tokenBuilder;
} LexerCtx;

typedef struct ParsingCtx {
    darray* tokens; //Token**
    darray* operatorStack; //Token**
    darray* outputQueue; //EvalNode**
} ParsingCtx;

void setCurrentId(Identifier newID, Identifier* id, LexerCtx* ctx);
