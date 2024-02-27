#ifndef BIGNUM_INTERNAL_H
#define BIGNUM_INTERNAL_H

#include "bignum/bignum.h"
#include "defines.h"

u32 computeUpperBound(const bignum* a, const bignum* b, i32* outOffsetA, i32* outOffsetB);

void addToArray(u32* array, u64 size, u64 idx, u32 num);
void singleWordMul(bignum* a, u32 b);
void trim(bignum* num);

u32 getWord(const bignum* num, u64 idx);
u16 getSmallWord(const bignum* num, u64 idx);

#endif /* ! BIGNUM_INTERNAL_H */
