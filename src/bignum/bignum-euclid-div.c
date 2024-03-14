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
#include <wctype.h>

#define MAX_ITER -10

#define RADIX 65536
#define RADIX_POWER 16
#define RADIX_MASK 65535

static void setSmallWord(bignum* num, u64 idx, u16 digit) {
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

static void appendSmallWord(bignum* num, u16 digit) {
    setSmallWord(num, num->size * 2, digit);
}

static void prependSmallWord(bignum* num, u16 digit) {
    prependWord(num, digit << 16);
}

static void product(bignum* x, const bignum* y, u16 k) {
    bnReset(x);
    u32 len = y->size * 2;
    u16 carry = 0;
    for (u32 i = y->unitWord * 2; i < len; i++) {
        u32 temp = getSmallWord(y, i) * k + carry;
        setSmallWord(x, i, temp & RADIX_MASK);
        carry = temp >> RADIX_POWER;
    }
    if (carry != 0)
        appendSmallWord(x, carry);
}

static void largeQuotient(bignum* num, u32 k) {
    u64 carry = 0;
    for (i32 i = num->size; i > num->unitWord; i--) {
        u64 temp = getWord(num, i - 1) + (((u64)carry) << 32);
        if (i <= 0)
            prependWord(num, temp / k);
        else
            num->words[i - 1] = temp / k;
        carry = temp % k;
    }
}

static inline u32 min(u32 a, u32 b) {
    return a < b ? a : b;
}

static i32 remain(bignum* y, u32 k) {
    u64 carry = 0;
    for (i32 i = y->size; i > y->unitWord; i--) {
        carry = (getWord(y, i - 1) + (((u64)carry) << 32)) % k;
    }
    return carry;
}

static u16 trial(bignum* r, bignum* d, u32 k, u32 m) {
    u64 km = k + m;
    u64 r3 = ((u64)getSmallWord(r, km) << (RADIX_POWER + RADIX_POWER)) + (getSmallWord(r, km - 1) << RADIX_POWER) +
             getSmallWord(r, km - 2);
    u64 d2 = ((u64)getSmallWord(d, m - 1 + d->unitWord * 2) << RADIX_POWER) + getSmallWord(d, m - 2 + d->unitWord * 2);
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
        i32 diff = getSmallWord(r, i + k + r->unitWord * 2) - getSmallWord(dq, i + dq->unitWord * 2) - borrow + RADIX;
        setSmallWord(r, i + k + r->unitWord * 2, diff % RADIX);
        borrow = 1 - diff / RADIX;
    }
    if (borrow != 0)
        setSmallWord(r, m + k + 1 + r->unitWord * 2, borrow + RADIX);
}

static void divideRemaining(bignum* x, bignum* r, bignum* dq, bignum* d, u64 m) {
    i32 max = MAX_ITER * 2;
    for (i32 l = -1; l >= max; l--) {
        i32 k = l & 1;
        if (k)
            prependSmallWord(r, 0);
        u16 qt = trial(r, d, k, m);
        product(dq, d, qt);
        if (smaller(r, dq, k, m)) {
            qt--;
            product(dq, d, qt);
        }
        if (qt == 0)
            break;
        if (k)
            prependSmallWord(x, qt);
        else
            setSmallWord(x, k, qt);
        difference(r, dq, k, m);
    }
}

static void longDivide(bignum* x, const bignum* y, bignum* r, bool euclidian) {

    u64 n = (x->size - x->unitWord) * 2;
    u64 m = (y->size - y->unitWord) * 2;
    u16 xmsd = getSmallWord(x, x->size * 2 - 1);
    if (xmsd == 0) {
        n--;
        xmsd = getSmallWord(x, x->size * 2 - 2);
    }
    u16 ymsd = getSmallWord(y, y->size * 2 - 1);
    if (ymsd == 0) {
        m--;
        ymsd = getSmallWord(y, y->size * 2 - 2);
    }

    bignum d; /** The normalized divisor */
    bignum dq;
    bnInit(&d);
    bnInit(&dq);
    bnCopy(x, r);
    bnCopy(y, &d);

    u16 f = RADIX / (ymsd + 1);
    bnMull(r, f);
    bnMull(&d, f);
    u64 allocLen = n - m + 1;
    allocLen += allocLen & 1;
    bnReserve(x, allocLen >> 1);

    i32 l = n - m;

    for (; l >= 0; l--) {
        u16 qt = trial(r, &d, l, m);
        bnCopy(&d, &dq);
        bnMull(&dq, qt);
        if (smaller(r, &dq, l, m)) {
            qt--;
            bnCopy(&d, &dq);
            bnMull(&dq, qt);
        }
        setSmallWord(x, l, qt);
        difference(r, &dq, l, m);
    }

    if (!euclidian)
        divideRemaining(x, r, &dq, &d, m);

    free(d.words);
    free(dq.words);
    if (euclidian) {
        largeQuotient(r, f);
        r->size = y->size - y->unitWord;
        trim(r);
    } else
        bnReset(r);
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

    if (divisor->size == 1 && divisor->unitWord == 0) {
        bnSet(outRemainder, remain(dividend, dCpy.words[0]));
        largeQuotient(dividend, dCpy.words[0]);
    } else if (bnCmp(dividend, divisor) < 0) {
        bnCopy(dividend, outRemainder);
        bnReset(dividend);
    } else
        longDivide(dividend, &dCpy, outRemainder, true);

    free(dCpy.words);

    if (negative)
        bnNeg(dividend);
}

i32 bnEuclidDivl(bignum* dividend, i32 divisor) {

    if (bnCmpl(dividend, 1) < 0 && bnCmpl(dividend, -1) > 0) {
        return 0;
    }

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
        prependWord(dividend, 0);
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

    bignum r;
    longDivide(dividend, divisor, &r, false);
    free(dCpy.words);

    if (negative)
        bnNeg(dividend);
}
