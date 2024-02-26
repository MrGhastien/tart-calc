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

#define RADIX 65536
#define RADIX_POWER 16
#define RADIX_MASK 65535

static u16 getWord(const bignum* num, u64 idx) {
    if (idx >= num->size * 2)
        return 0;
    return ((u16*)num->words)[idx];
}

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

static void product(bignum* x, const bignum* y, u16 k) {
    u32 len = y->size * 2;
    u16 carry = 0;
    for (u32 i = 0; i < len; i++) {
        u32 temp = getWord(y, i) * k + carry;
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
        u32 temp = getWord(num, i - 1) + (carry << RADIX_POWER);
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

static i32 remainder(bignum* y, u32 k) {
    u32 carry = 0;
    for (u32 i = y->size; i > 0; i--) {
        carry = (y->words[i - 1] + (((u64)carry) << 32)) % k;
    }
    return carry;
}

static u16 trial(bignum* r, bignum* d, u32 k, u32 m) {
    u64 km = k + m;
    u64 r3 =
        ((u64)getWord(r, km) << (RADIX_POWER + RADIX_POWER)) + (getWord(r, km - 1) << RADIX_POWER) + getWord(r, km - 2);
    u64 d2 = ((u64)getWord(d, m - 1) << RADIX_POWER) + getWord(d, m - 2);
    return min(r3 / d2, RADIX - 1);
}

static bool smaller(bignum* r, bignum* dq, u32 k, u32 m) {
    u32 i = m;
    u32 j = 0;
    while (i != j) {
        if (getWord(r, i + k) != getWord(dq, i))
            j = i;
        else
            i--;
    }
    return getWord(r, i + k) < getWord(dq, i);
}

static void difference(bignum* r, bignum* dq, u32 k, u32 m) {
    i32 borrow = 0;
    for (u32 i = 0; i <= m; i++) {
        i32 diff = getWord(r, i + k) - getWord(dq, i) - borrow + RADIX;
        setWord(r, i + k, diff % RADIX);
        borrow = 1 - diff / RADIX;
    }
}

static void longdivide(bignum* x, const bignum* y, bignum* r) {

    u64 n = x->size * 2;
    u64 m = y->size * 2;
    u16 xmsd = getWord(x, n - 1);
    if (xmsd == 0) {
        n--;
        xmsd = getWord(x, n - 1);
    }
    u16 ymsd = getWord(y, m - 1);
    if (ymsd == 0) {
        m--;
        ymsd = getWord(y, m - 1);
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
    free(d.words);
    free(dq.words);
    trim(x);
    trim(r);
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
        bnInit(outRemainder);
        bnSet(outRemainder, remainder(dividend, dCpy.words[0]));
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

void bnEuclidDivl(bignum* dividend, i32 divisor, i32* outRemainder) {
    bool negative = false;
    if (bnSign(dividend) < 0) {
        bnNeg(dividend);
        negative = true;
    }

    if (divisor < 0) {
        divisor = -divisor;
        negative = !negative;
    }

    *outRemainder = remainder(dividend, divisor);
    largeQuotient(dividend, divisor);

    if (negative)
        bnNeg(dividend);
}

void bnDivl(bignum* divident, i32 divisor, i32* outRemainder) {
    
}
