#include "bignum-internal.h"
#include "bignum/bignum.h"
#include "error.h"
#include "util.h"

#include <stdlib.h>

#define ALLONE 0xffffffff

static bool canIgnoreWord(u64 idx, u32* array) {
    u32 word = array[idx];
    if (idx == 0)
        return word == 0;
    if (word != 0 && word != ALLONE)
        return false;

    u32 sign = array[idx - 1] >> 31;
    if (word == 0 && sign == 1)
        return false;
    if (word == ALLONE && sign == 0)
        return false;
    return true;
}

void addToArray(u32* array, u64 size, u64 idx, u32 num) {
    if (idx >= size)
        return;

    i64 workspace = array[idx];
    workspace += num;

    array[idx] = workspace & ALLONE;
    if (idx + 1 < size)
        array[idx + 1] = workspace >> bitsizeof(u32);
}

void trim(bignum* num) {
    if (num->size == 0)
        return;
    u64 trailing = 0;
    u64 leading = 0;
    while (trailing < num->size && (num->words[trailing] == 0)) {
        trailing++;
    }

    while (leading < num->size - trailing && canIgnoreWord(num->size - leading - 1, num->words)) {
        leading++;
    }

    u64 newSize = num->size - trailing - leading;

    if (newSize != num->size) {
        if (newSize == 0) {
            bnReset(num);
            return;
        }

        if (trailing != 0) {
            for (u64 i = 0; i < newSize; i++) {
                num->words[i] = num->words[i + trailing];
            }
        }

        u32* newArray = realloc(num->words, newSize * sizeof(*newArray));
        if (!newArray)
            return;
        num->words = newArray;
        num->size = newSize;
        num->unitWord -= trailing;
    }
}

u32 computeUpperBound(const bignum* a, const bignum* b, i32* outOffsetA, i32* outOffsetB) {
    i32 offset = max(a->unitWord, b->unitWord);
    *outOffsetA = offset - a->unitWord;
    *outOffsetB = offset - b->unitWord;

    return max(*outOffsetA + a->size, *outOffsetB + b->size);
}

static void bnAddInternal(bignum* a, const bignum* b, i32 carry, bool not ) {
    if (not )
        carry += 1;

    i32 offseta;
    i32 offsetb;
    u32 upperBound = computeUpperBound(a, b, &offseta, &offsetb);
    i32 signA = bnSign(a);

    u32* newArray = calloc(upperBound + 1, sizeof *newArray);
    if (!newArray) {
        signalErrorNoToken(ERR_ALLOC_FAIL, NULL, -1);
        return;
    }

    // u64 lastSign = 0;
    for (u32 k = 0; k < upperBound; k++) {
        u32 wordA = getWord(a, k - offseta);
        u32 wordB = getWord(b, k - offsetb);
        if (not)
            wordB = ~wordB;

        // Do this in 3 steps instead of 1 to ensure that
        // there is no 32 bit overflow
        u64 workspace = wordA;
        workspace += wordB;
        workspace += carry;

        newArray[k] = workspace & 0xffffffff;
        carry = workspace >> bitsizeof(u32);
        // lastSign = newArray[k] >> (bitsizeof(u32) - 1);
    }

    newArray[upperBound] = carry;
    free(a->words);
    a->words = newArray;
    a->size = upperBound + 1;
    a->unitWord = max(a->unitWord, b->unitWord);
    trim(a);
}

void bnAdd(bignum* a, const bignum* b) {
    bnAddInternal(a, b, 0, false);
}

void bnAddl(bignum* a, i32 b) {
    bignum tmp;
    bnInit(&tmp);
    bnSet(&tmp, b);
    bnAddInternal(a, &tmp, 0, false);
    free(tmp.words);
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
    dst->size = src->size;
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
    u32 upperBound = a->size + b->size + 1;

    u32* newArray = calloc(upperBound, sizeof *newArray);
    if (!newArray) {
        signalErrorNoToken(ERR_ALLOC_FAIL, NULL, -1);
        return;
    }

    for (u64 i = 0; i < b->size; i++) {
        i32 wordB = b->words[i];
        if (wordB == 0)
            continue;

        for (u64 j = 0; j < a->size; j++) {
            i32 wordA = a->words[j];
            if (wordA == 0)
                continue;

            i64 workspace = wordA;
            workspace *= wordB;

            u64 idx = i + j;
            addToArray(newArray, upperBound, idx, workspace & ALLONE);
            addToArray(newArray, upperBound, idx + 1, workspace >> bitsizeof(u32));
        }
    }
    free(a->words);
    a->words = newArray;
    a->unitWord = a->unitWord + b->unitWord;
    a->size = upperBound;
    trim(a);
}

void singleWordMul(bignum* a, u32 b) {
    u64 uppderBound = a->size + 1;
    u32* tmp = calloc(uppderBound, sizeof *tmp);
    if (!tmp) {
        signalErrorNoToken(ERR_ALLOC_FAIL, NULL, -1);
        return;
    }

    for (u64 i = 0; i < a->size; i++) {
        u32 wordA = getWord(a, i);
        i64 workspace = wordA;
        workspace *= b;

        u64 idx = i;
        addToArray(tmp, uppderBound, idx, workspace & ALLONE);
        addToArray(tmp, uppderBound, idx + 1, workspace >> bitsizeof(u32));
    }
    free(a->words);
    a->words = tmp;
    a->size = uppderBound;
    trim(a);
}

void bnMul(bignum* a, const bignum* b) {
    // u64 mid = max(a->size, b->size) / 2;
    if (b->size == 1)
        singleWordMul(a, b->words[0]);
    else
        simpleMul(a, b);
}

void bnMull(bignum* a, i32 b) {
    singleWordMul(a, b);
}

// static void karatsuba(u32* awords, u32 alen, u32*
