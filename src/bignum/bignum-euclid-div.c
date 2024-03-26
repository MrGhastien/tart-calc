/**
 * Implementation of the division algorithm presented by Per Brinch Hansen,
 * at the Syracuse University.
 * See https://surface.syr.edu/cgi/viewcontent.cgi?article=1162&context=eecs_techreports
 */

#include "bignum-internal.h"
#include "bignum/bignum.h"
#include "error.h"
#include <stdlib.h>
#include <string.h>

#define MAX_ITER -10

#define RADIX 65536
#define RADIX_POWER 16
#define RADIX_MASK 65535

typedef struct div_info_t {
    bignum* r;
    bignum* d;
    bignum* dq;
    u64 n;
    u64 m;
    u32 sizea;
    u32 sizeb;
    i32 offseta;
    i32 offsetb;
    u32 f;
} div_info;

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

static void prependSmallWord(bignum* num, u16 digit) {
    prependWord(num, digit << 16);
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
        carry = getWord(y, i - 1) + (carry << 32);
        carry %= k;
    }
    return carry;
}

/*
 * Divide the 3 MSDs of the remainder by the 2 MSDs of the divisor.
 * This results in a estimate of a quotient digit.
 */
static u16 trial(div_info* info, i32 l) {
    u64 idx = l + info->m + info->offseta;
    u64 r3 = getSmallWord(info->r, idx);
    r3 <<= RADIX_POWER;
    r3 |= getSmallWord(info->r, idx - 1);
    r3 <<= RADIX_POWER;
    r3 |= getSmallWord(info->r, idx - 2);

    u64 d2 = getSmallWord(info->d, info->d->size * 2 - 1);
    d2 <<= RADIX_POWER;
    d2 |= getSmallWord(info->d, info->d->size * 2 - 2);
    return min(r3 / d2, RADIX - 1);
}

static bool smaller(bignum* r, bignum* dq, u32 k, u32 m) {
    u32 i = m;
    u32 j = 0;
    u32 dqIdx = i - dq->unitWord * 2;
    while (i != j) {
        if (getSmallWord(r, i + k) != getSmallWord(dq, dqIdx))
            j = i;
        else
            i--;
    }
    return getSmallWord(r, i + k) < getSmallWord(dq, dqIdx);
}

static void difference(bignum* r, bignum* dq, u32 k, u32 m) {
    i32 borrow = 0;
    k += r->unitWord * 2;
    for (u32 i = 0; i <= m; i++) {
        i32 diff = getSmallWord(r, i + k) - getSmallWord(dq, i + dq->unitWord * 2) - borrow + RADIX;
        setSmallWord(r, i + k, diff % RADIX);
        borrow = 1 - diff / RADIX;
    }
    if (borrow != 0)
        setSmallWord(r, m + k + 1, borrow + RADIX);
}

static void divideRemaining(bignum* x, div_info* info, i32 l) {
    i32 max = MAX_ITER * 2;
    for (; l >= -max; l--) {
        i32 k = l & 1;
        if (k)
            prependSmallWord(info->r, 0);
        u16 qt = trial(info, l);
        bnCopy(info->dq, info->d);
        bnMull(info->dq, qt);
        if (smaller(info->r, info->dq, k, info->m)) {
            qt--;
            bnSub(info->dq, info->d);
        }
        if (qt == 0)
            break;
        if (k)
            prependSmallWord(x, qt);
        else
            setSmallWord(x, k, qt);
        //difference(info->r, dq, k, m);
        bnSub(info->r, info->dq);
        if (bnCmpl(info->r, 0) == 0)
            break;
    }
}

static void longDivide(bignum* x, const bignum* y, bignum* r, bool euclidian) {

    bignum normDivisor; /** The nor    u64 allocLen = info.n - info.m;
    allocLen += allocLen & 1;
malized divisor */
    bignum prodDivisor;

    div_info info = {
        .n = countWords(x) * 2,
        .m = countWords(y) * 2,
        .offseta = x->unitWord * 2,
        .offsetb = y->unitWord * 2,
        .sizeb = y->size * 2,
        .sizea = x->size * 2,
        .r = r,
        .d = &normDivisor,
        .dq = &prodDivisor,
    };

    if (getSmallWord(x, info.sizea - 1) == 0) {
        info.n--;
        info.sizea--;
    }
    u16 ymsd = getSmallWord(y, info.sizeb - 1);
    if (ymsd == 0) {
        info.sizeb--;
        info.m--;
        ymsd = getSmallWord(y, info.m - 1);
    }

    bnInit(&normDivisor);
    bnInit(&prodDivisor);
    bnCopy(x, r);
    bnCopy(y, &normDivisor);

    u16 f = RADIX / (ymsd + 1);
    bnMull(r, f);
    bnMull(&normDivisor, f);

    u64 allocLen = x->size - y->size + y->unitWord - x->unitWord;
    bnReserve(x, allocLen);

    /* index of the current quotient digit */
    i32 l = info.n - info.m - 1;

    for (; l >= 0; l--) {
        u16 quotDigit = trial(&info, l);
        bnCopy(&normDivisor, &prodDivisor);
        bnMull(&prodDivisor, quotDigit);
        if (smaller(r, &prodDivisor, l, m)) {
            quotDigit--;
            bnSub(&prodDivisor, &normDivisor);
        }
        setSmallWord(x, l, quotDigit);
        //difference(r, &prodDivisor, l, m);
        bnSub(r, &prodDivisor);
    }

    if (!euclidian)
        divideRemaining(x, r, &prodDivisor, &normDivisor, m, l);

    free(normDivisor.words);
    free(prodDivisor.words);
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
