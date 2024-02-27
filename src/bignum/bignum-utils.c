#include "bignum-internal.h"

u32 getWord(const bignum* num, u64 idx) {
    if (idx < 0 || idx >= num->size)
        return 0;
    return num->words[idx];
}

u16 getSmallWord(const bignum* num, u64 idx) {
    if (idx >= num->size * 2)
        return 0;
    return ((u16*)num->words)[idx];
}
