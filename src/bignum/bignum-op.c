#include "bignum/bignum.h"
#include "error.h"
#include "util.h"

#include <stdlib.h>

#define ALLONE 0xffffffff

static u32 getWord(const bignum* num, long idx) {
    if(idx < 0 || idx >= num->size)
        return 0;
    return num->words[idx];
}

static bool canIgnoreWord(u32 word, u32* array, u64 arraySize) {
    u32 sign = array[arraySize - 1] >> 31;
    if((word == 0) && sign == 1)
        return false;
    if((word == ALLONE) && sign == 0)
        return false;
    return true;
}

static void trim(bignum* num) {
    if(num->size == 0)
        return;
    i64 trailing = 0;
    i64 leading = num->size;
    while (trailing < num->size && (num->words[trailing] == 0)) {
        trailing++;
        leading--;
    }

    while(leading > 1 && canIgnoreWord(num->words[leading], num->words, num->size)) {
        leading--;
    }

    if(trailing != 0) {
        for (i64 i = 0; i < leading - trailing; i++) {
            num->words[i] = num->words[i + trailing];
        }
    }

    if (leading != num->size) {
        u32* newArray = realloc(num->words, leading - trailing);
        if(!newArray)
            return;
        num->words = newArray;
    }

    num->size = leading - trailing;
    num->unitWord -= trailing;
}

static u32 computeUpperBound(const bignum* a, const bignum* b, i32* outOffsetA, i32* outOffsetB) {
    i32 offset = max(a->unitWord, b->unitWord);
    *outOffsetA = offset - a->unitWord;
    *outOffsetB = offset - b->unitWord;

   return max(*outOffsetA + a->size, *outOffsetB + b->size);
}

static void bnAddInternal(bignum* a, const bignum* b, i32 carry, bool not) {
    if(not)
        carry += 1;

    i32 offseta;
    i32 offsetb;
    u32 upperBound = computeUpperBound(a, b, &offseta, &offsetb);

    u32* newArray = calloc(upperBound, sizeof *newArray);
    if (!newArray) {
        signalErrorNoToken(ERR_ALLOC_FAIL, NULL, -1);
        return;
    }

    for (u32 k = 0; k < upperBound; k++) {
        i32 wordA = getWord(a, k - offseta);
        i32 wordB = getWord(b, k - offsetb);
        if(not)
            wordB = ~wordB;

        // Do this in 3 steps instead of 1 to ensure that
        // there is no 32 bit overflow
        i64 workspace = wordA;
        workspace += wordB;
        workspace += carry;

        newArray[k] = workspace & 0xffffffff;
        carry = workspace >> 32;
    }

    if (!canIgnoreWord(carry, newArray, upperBound)) {
        newArray = realloc(newArray, (upperBound + 1) * sizeof *a->words);
        if (!newArray) {
            signalErrorNoToken(ERR_ALLOC_FAIL, NULL, -1);
            return;
        }
        newArray[upperBound] = carry;
        upperBound++;
    }
    free(a->words);
    a->words = newArray;
    a->size = upperBound;
    trim(a);
}

void bnAdd(bignum* a, const bignum* b) {
    bnAddInternal(a, b, 0, false);
}

void bnSub(bignum* a, const bignum* b) {
    bnAddInternal(a, b, 0, true);
}

void bnNot(bignum* num) {
    for (u32 i = 0; i < num->size; i++) {
        num->words[i] = ~num->words[i];
    }
}

static void bnMove(bignum* src, bignum* dst) {
    free(dst->words);
    dst->size =  src->size;
    dst->unitWord = src->unitWord;
    dst->words = src->words;
}

void bnNeg(bignum* num) {
    bignum t;
    bnInit(&t);
    bnAddInternal(&t, num, 0, true);
    bnMove(&t, num);
}

static void simpleMul(bignum* a, const bignum* b) {
    u32* tmp = calloc(a->size + b->size, sizeof *tmp);
    if (!tmp) {
        signalErrorNoToken(ERR_ALLOC_FAIL, NULL, -1);
        return;
    }

    i32 offseta;
    i32 offsetb;
    computeUpperBound(a, b, &offseta, &offsetb);
    u32 upperBound = a->size + b->size;

    u32* newArray = calloc(upperBound + 1, sizeof *newArray);
    if (!newArray) {
        signalErrorNoToken(ERR_ALLOC_FAIL, NULL, -1);
        return;
    }

    for (u64 i = 0; i < a->size; i++) {
        i32 wordA = getWord(a, i - offseta);
        if (wordA == 0)
            continue;

        for (u64 j = 0; j < b->size; j++) {
            i32 wordB = getWord(b, j - offsetb);
            if (wordA == 0)
                continue;

            i64 workspace = wordA;
            workspace *= wordB;

            u64 idx = (i - offseta) + (j - offsetb);
            newArray[idx] = workspace & ALLONE;
            newArray[idx + 1] = workspace >> bitsizeof(u32);
        }
    }
    free(a->words);
    a->words = newArray;
    a->size = upperBound;
    trim(a);
}

void bnMul(bignum* a, const bignum* b) {
    //u64 mid = max(a->size, b->size) / 2;
    simpleMul(a, b);
}

//static void karatsuba(u32* awords, u32 alen, u32* 
