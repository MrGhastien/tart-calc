#ifndef BIGNUM_INTERNAL_H
#define BIGNUM_INTERNAL_H

#include "bignum/bignum.h"
#include "defines.h"

u32 computeUpperBound(const bignum* a, const bignum* b, i32* outOffsetA, i32* outOffsetB);

void addToArray(u32* array, u64 size, u64 idx, u32 num);
void trim(bignum* num);

u32 getWord(const bignum* num, i32 idx);
u16 getSmallWord(const bignum* num, u64 idx);

void setWord(bignum* num, u64 idx, u32 digit);

void appendWord(bignum* num, u16 digit);
void prependWord(bignum* num, u32 word);

u32 countWords(const bignum* num);

#endif /* ! BIGNUM_INTERNAL_H */
