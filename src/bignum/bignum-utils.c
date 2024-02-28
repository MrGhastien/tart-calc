#include "bignum-internal.h"

u32 getWord(const bignum* num, u64 idx) {
    if (idx >= num->size)
        return 0;
    return num->words[idx];
}

u16 getSmallWord(const bignum* num, u64 idx) {
    if (idx >= num->size * 2)
        return 0;
    return ((u16*)num->words)[idx];
}

u64 nextPower(u64 num) {
    num--;
    num |= num >> 1;
    num |= num >> 2;
    num |= num >> 4;
    num |= num >> 8;
    num |= num >> 16;
    num |= num >> 32;
    return num - 1;
}
