/**
 * Implementation of the division algorithm presented by Per Brinch Hansen,
 * at the Syracuse University.
 * See https://surface.syr.edu/cgi/viewcontent.cgi?article=1162&context=eecs_techreports
 */

#include "bignum-internal.h"
#include "bignum/bignum.h"
#include "error.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>

#define MAX_ITER -10

#define RADIX 65536
#define RADIX_POWER 16
#define RADIX_MASK 65535

static void setWord(bignum* num, u64 idx, u16 digit) {
    if (idx == num->size * 2) {
        u32* buf = realloc(num->words, sizeof *buf * (num->size + 1));
        if (!buf) {
            signalErrorNoToken(ERR_ALLOC_FAIL, NULL, -1);
            return;
        }
        num->words = buf;
        num->size++;
    }
    ((u16*)num->words)[idx] = digit;
}

static void appendWord(bignum* num, u16 digit) {
    setWord(num, num->size * 2, digit);
}

static void push_word(bignum* num, u32 word) {
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

static void prepend_small_word(bignum* num, u16 digit) {
    push_word(num, digit << 16);
}

static void product(bignum* x, const bignum* y, u16 k) {
    bnReset(x);
    u32 len = y->size * 2;
    u16 carry = 0;
    for (u32 i = 0; i < len; i++) {
        u32 temp = getSmallWord(y, i) * k + carry;
        setWord(x, i, temp & RADIX_MASK);
        carry = temp >> RADIX_POWER;
    }
    if (carry != 0)
        appendWord(x, carry);
}

static void quotient(bignum* num, u16 k) {
    u32 len = num->size * 2;
    u16 carry = 0;
    for (u32 i = len; i > 0; i--) {
        u32 temp = getSmallWord(num, i - 1) + (carry << RADIX_POWER);
        setWord(num, i - 1, temp / k);
        carry = temp % k;
    }
}

static void largeQuotient(bignum* num, u32 k) {
    u32 carry = 0;
    for (u32 i = num->size; i > 0; i--) {
        u64 temp = num->words[i - 1] + (((u64)carry) << 32);
        num->words[i - 1] = temp / k;
        carry = temp % k;
    }
}

static inline u32 min(u32 a, u32 b) {
    return a < b ? a : b;
}

static i32 remain(bignum* y, u32 k) {
    u32 carry = 0;
    for (u32 i = y->size; i > 0; i--) {
        carry = (y->words[i - 1] + (((u64)carry) << 32)) % k;
    }
    return carry;
}

static u16 trial(bignum* r, bignum* d, u32 k, u32 m) {
    u64 km = k + m;
    u64 r3 = ((u64)getSmallWord(r, km) << (RADIX_POWER + RADIX_POWER)) + (getSmallWord(r, km - 1) << RADIX_POWER) +
             getSmallWord(r, km - 2);
    u64 d2 = ((u64)getSmallWord(d, m - 1) << RADIX_POWER) + getSmallWord(d, m - 2);
    return min(r3 / d2, RADIX - 1);
}

static bool smaller(bignum* r, bignum* dq, u32 k, u32 m) {
    u32 i = m;
    u32 j = 0;
    while (i != j) {
        if (getSmallWord(r, i + k) != getSmallWord(dq, i))
            j = i;
        else
            i--;
    }
    return getSmallWord(r, i + k) < getSmallWord(dq, i);
}

static void difference(bignum* r, bignum* dq, u32 k, u32 m) {
    i32 borrow = 0;
    for (u32 i = 0; i <= m; i++) {
        i32 diff = getSmallWord(r, i + k) - getSmallWord(dq, i) - borrow + RADIX;
        setWord(r, i + k, diff % RADIX);
        borrow = 1 - diff / RADIX;
    }
    if (borrow != 0)
        setWord(r, m + k + 1, borrow + RADIX);
}

static void longdivide(bignum* x, const bignum* y, bignum* r) {

    u64 n = x->size * 2;
    u64 m = y->size * 2;
    u16 xmsd = getSmallWord(x, n - 1);
    if (xmsd == 0) {
        n--;
        xmsd = getSmallWord(x, n - 1);
    }
    u16 ymsd = getSmallWord(y, m - 1);
    if (ymsd == 0) {
        m--;
        ymsd = getSmallWord(y, m - 1);
    }

    bignum d; /** The normalized divisor */
    bignum dq;
    bnInit(&d);
    bnInit(&dq);

    u16 f = RADIX / (ymsd + 1);
    product(r, x, f);
    product(&d, y, f);
    u64 allocLen = n - m + 1;
    allocLen += allocLen & 1;
    u16* buf = calloc(allocLen, sizeof *buf);
    if (!buf) {
        signalErrorNoToken(ERR_ALLOC_FAIL, NULL, -1);
        return;
    }

    for (i32 k = n - m; k >= 0; k--) {
        u16 qt = trial(r, &d, k, m);
        product(&dq, &d, qt);
        if (smaller(r, &dq, k, m)) {
            qt--;
            product(&dq, &d, qt);
        }
        buf[k] = qt;
        difference(r, &dq, k, m);
    }
    free(x->words);
    x->words = (u32*)buf;
    x->size = allocLen >> 1;
    quotient(r, f);
    r->size = y->size;
    free(d.words);
    free(dq.words);
    trim(x);
    trim(r);
}

static void precisedivide(bignum* x, const bignum* y) {

    u64 n = x->size * 2;
    u64 m = y->size * 2;
    u16 xmsd = getSmallWord(x, n - 1);
    if (xmsd == 0) {
        n--;
        xmsd = getSmallWord(x, n - 1);
    }
    u16 ymsd = getSmallWord(y, m - 1);
    if (ymsd == 0) {
        m--;
        ymsd = getSmallWord(y, m - 1);
    }

    bignum d; /** The normalized divisor */
    bignum dq;
    bignum r;
    bnInit(&r);
    bnInit(&d);
    bnInit(&dq);

    u16 f = RADIX / (ymsd + 1);
    product(&r, x, f);
    product(&d, y, f);
    u64 allocLen = n - m + 1;
    allocLen += allocLen & 1;
    bnReserve(x, allocLen >> 1);

    i32 l = n - m;

    for (; l >= 0; l--) {
        u16 qt = trial(&r, &d, l, m);
        product(&dq, &d, qt);
        if (smaller(&r, &dq, l, m)) {
            qt--;
            product(&dq, &d, qt);
        }
        setWord(x, l, qt);
        difference(&r, &dq, l, m);
    }

    i32 max = MAX_ITER * 2;
    for (; l >= max; l--) {
        i32 k = l & 1;
        if (k)
            prepend_small_word(&r, 0);
        u16 qt = trial(&r, &d, k, m);
        product(&dq, &d, qt);
        if (smaller(&r, &dq, k, m)) {
            qt--;
            product(&dq, &d, qt);
        }
        if (qt == 0)
            break;
        if (k)
            prepend_small_word(x, qt);
        else
            setWord(x, k, qt);
        difference(&r, &dq, k, m);
    }
    free(d.words);
    free(dq.words);
    free(r.words);
    trim(x);
}

void bnEuclidDiv(bignum* dividend, const bignum* divisor, bignum* outRemainder) {
    i32 offsetA;
    i32 offsetB;
    computeUpperBound(dividend, divisor, &offsetA, &offsetB);
    bignum dCpy;
    bnInit(&dCpy);
    bnCopy(divisor, &dCpy);

    bool negative = false;
    if (bnSign(dividend) < 0) {
        bnNeg(dividend);
        negative = true;
    }

    if (bnSign(divisor) < 0) {
        bnNeg(&dCpy);
        negative = !negative;
    }

    if (divisor->size == 1) {
        bnSet(outRemainder, remain(dividend, dCpy.words[0]));
        largeQuotient(dividend, dCpy.words[0]);
    } else if (divisor->size - offsetB > dividend->size - offsetA) {
        bnCopy(dividend, outRemainder);
        bnReset(dividend);
    } else
        longdivide(dividend, &dCpy, outRemainder);

    free(dCpy.words);

    if (negative)
        bnNeg(dividend);
}

i32 bnEuclidDivl(bignum* dividend, i32 divisor) {
    bool negative = false;
    if (bnSign(dividend) < 0) {
        bnNeg(dividend);
        negative = true;
    }

    if (divisor < 0) {
        divisor = -divisor;
        negative = !negative;
    }

    i32 mod = remain(dividend, divisor);
    largeQuotient(dividend, divisor);

    if (negative)
        bnNeg(dividend);
    return mod;
}

void bnDivl(bignum* dividend, i32 divisor) {
    bool negative = false;
    if (bnSign(dividend) < 0) {
        bnNeg(dividend);
        negative = true;
    }

    if (divisor < 0) {
        divisor = -divisor;
        negative = !negative;
    }

    u32 carry = 0;
    i32 i = dividend->size;
    while (i > 0) {
        u64 temp = dividend->words[i - 1] + (((u64)carry) << 32);
        dividend->words[i - 1] = temp / divisor;
        carry = temp % divisor;
        i--;
    }

    while (carry != 0 && i > MAX_ITER) {
        push_word(dividend, 0);
        u64 temp = dividend->words[0] + (((u64)carry) << 32);
        dividend->words[0] = temp / divisor;
        carry = temp % divisor;
        i--;
    }

    if (negative)
        bnNeg(dividend);
    trim(dividend);
}

void bnDiv(bignum* dividend, const bignum* divisor) {
    bignum dCpy;
    bnInit(&dCpy);
    bnCopy(divisor, &dCpy);

    bool negative = false;
    if (bnSign(dividend) < 0) {
        bnNeg(dividend);
        negative = true;
    }

    if (bnSign(divisor) < 0) {
        bnNeg(&dCpy);
        negative = !negative;
    }

    precisedivide(dividend, divisor);

    free(dCpy.words);

    if (negative)
        bnNeg(dividend);
}
