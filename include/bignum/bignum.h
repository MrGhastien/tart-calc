#ifndef BIGNUM_H
#define BIGNUM_H

#include "defines.h"

typedef struct bignum {
    u32* words;
    u32 size; // Number of words.
    i32 unitWord; // Index of the unit word inside the array. May be out of bounds.
} bignum;

/* Initializes a new bignum. */
void bnInit(bignum* num);

/* Sets a bignum to the given integer value */
void bnSet(bignum* num, long value);

/* Clears and initializes a bignum. */
void bnReset(bignum* num);

long bnAsLong(bignum* num);
double bnAsDouble(bignum* num);

void bnCopy(const bignum* src, bignum* dst);

/* Negates a bignum *in place*. */
void bnNot(bignum* num);
/* Inverts all the bits of a bignum *in place*. */
void bnNeg(bignum* num);

void bnAdd(bignum* a, const bignum* b);
void bnSub(bignum* a, const bignum* b);
void bnMul(bignum* a, const bignum* b);
void bnDiv(bignum* a, const bignum* b);
void bnMod(bignum* a, const bignum* b);
void bnPow(bignum* a, const bignum* b);

int bnCmp(const bignum* a, const bignum* b);
int bnCmpl(const bignum* a, long b);

i64 bnStr(const bignum* num, char** outStr);

#endif /* ! BIGNUM_H */