#include "bignum-internal.h"
#include "bignum/bignum.h"
#include "error.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>

u32 getWord(const bignum* num, i32 idx) {
    if (idx < 0)
        return 0;
    if (idx >= num->size)
        return bnSign(num) < 0 ? 0xffffffff : 0;
    return num->words[idx];
}

u16 getSmallWord(const bignum* num, u64 idx) {
    if (idx >= num->size * 2)
        return 0;
    return ((u16*)num->words)[idx];
}

void setWord(bignum* num, u64 idx, u32 digit) {
    if (idx == num->size) {
        u32* buf = realloc(num->words, sizeof *buf * (num->size + 1));
        if (!buf) {
            signalErrorNoToken(ERR_ALLOC_FAIL, NULL, -1);
            return;
        }
        num->words = buf;
        num->size++;
    }
    num->words[idx] = digit;
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

void appendWord(bignum* num, u16 digit) {
    setWord(num, num->size, digit);
}

void prependWord(bignum* num, u32 word) {
    u32* array = realloc(num->words, sizeof *array * (num->size + 1));
    if (!array) {
        signalErrorNoToken(ERR_ALLOC_FAIL, NULL, -1);
        return;
    }

    memmove(array + 1, array, num->size * sizeof(*array));
    array[0] = word;

    num->words = array;
    num->size++;
    num->unitWord++;
}

/*
 * Get the number of digits of a number.
 */
u32 countWords(const bignum* num) {
    if (num->unitWord < 0)
        return num->size - num->unitWord;
    return num->size;
}
