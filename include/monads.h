#include "defines.h"
#include "token.h"

#define DEF_RESULT(suffix, type)                                           \
    typedef struct {                                                                                                   \
        type value;                                                     \
        bool ok;                                                                                                       \
    } Result##suffix;                                                   \
    \
    Result##suffix resultOf##suffix(type value)

#define DEF_RESULT_SIMPLE(type) DEF_RESULT(type, type)

DEF_RESULT_SIMPLE(u64);
DEF_RESULT(Tokens, Token**);
