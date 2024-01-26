#include "bignum/bignum.h"
#include "util.h"
#include "error.h"

#include <stdlib.h>

void bnInit(bignum* num) {
    num->words = NULL;
    num->size = 0;
    num->unitWord = 0;
}

void bnReset(bignum* num) {
    if (num->words)
        free(num->words);
    bnInit(num);
}

void bnSet(bignum* num, long value) {
    if (value == 0) {
        bnReset(num);
        return;
    }

    u64 bits = value;
    num->unitWord = 0;
    u32 newSize;
    u32 second = bits >> 32;
    u32 first = bits & 0xffffffff;
    if(second != 0 && second != 0xffffffff)
        newSize = 2;
    else
        newSize = 1;
    u32* newArray = realloc(num->words, newSize * sizeof *num->words);
    if(!newArray) {
        signalErrorNoToken(ERR_ALLOC_FAIL, NULL, -1);
        return;
    }
    num->words = newArray;
    num->size = newSize;
    if (newSize == 2)
        num->words[1] = second;
    num->words[0] = first;
}

void bnCopy(const bignum* src, bignum* dst) {
    u32* newWords = realloc(dst->words, src->size * sizeof *dst->words);
    if (!newWords) {
        signalErrorNoToken(ERR_ALLOC_FAIL, NULL, -1);
        return;
    }
    memcpy(newWords, dst->words, src->size * sizeof *dst->words);
    dst->words = newWords;
    dst->size = src->size;
    dst->unitWord = src->unitWord;
}

int bnSign(const bignum* num) {
    if(num->size == 0)
        return 0;
    if(num->words[num->size - 1] >> 31)
        return -1;
    return 1;
}

int bnCmp(const bignum* a, const bignum* b) {
    int signA = bnSign(a);
    int signB = bnSign(b);
    if(signA != signB)
        return signA;

    u32 otherOffset = a->unitWord - b->unitWord;
    int comp = 0;
    for (u64 i = 0; i < a->size; i++) {
        u32 word = a->words[i];
        u32 otherIndex = i - otherOffset;
        u32 otherWord;
        if (otherIndex < 0 || otherIndex > b->size) {
            otherWord = 0;
        } else {
            otherWord = b->words[otherIndex];
        }
        if (word < otherWord) {
            comp = -1;
            break;
        } else if (word > otherWord) {
            comp = 1;
            break;
        }
    }
    return comp * signA;
}

int bnCmpl(const bignum* a, long b) {
    if(a->size > 2)
        return bnSign(a);

    long fword = a->size == 1 ? 0 : a->words[1];
    long sword = a->size == 0 ? 0 : a->words[0];

    long sum = fword << (bitsizeof(*a->words)) | sword;
    return b - sum;
}

i64 bnStr(const bignum* num, char** outStr) {
    size_t len = 0;
    return 0;
}