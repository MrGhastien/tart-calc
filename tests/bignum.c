#include "bignum/bignum.h"
#include <criterion/criterion.h>
#include <stdio.h>

TestSuite(BugNumSuite);

Test(BigNumSuite, bignum_init) {
    bignum num;
    bnInit(&num);

    bignum num2;
    bnInit(&num2);
    bnSet(&num2, 0);
    
    cr_expect_eq(bnCmp(&num, &num2), 0);
}

Test(BigNumSuite, bignum_neg) {
    bignum num;
    bnInit(&num);

    bnSet(&num, 17);
    bnNeg(&num);

    bignum num2;
    bnInit(&num2);
    bnSet(&num2, -17);
    cr_expect_eq(bnCmp(&num, &num2), 0);

    bnSet(&num, 0xff78ad82fced);
    bnNeg(&num);
    cr_expect_eq(bnCmpl(&num, -0xff78ad82fced), 0);

    free(num.words);
    free(num2.words);
}

Test(BigNumSuite, bignum_add) {
    bignum num;
    bignum num2;
    bnInit(&num);
    bnInit(&num2);

    bnSet(&num, 17);
    bnSet(&num2, 100);

    bnAdd(&num, &num2);
    cr_expect_eq(bnCmpl(&num, 117), 0);

    bnSet(&num, -100);
    bnSet(&num2, 17);
    bnAdd(&num, &num2);
    cr_expect_eq(bnCmpl(&num, -83), 0);

    bnSet(&num, -100);
    bnSet(&num2, 100);
    bnAdd(&num, &num2);
    cr_expect_eq(bnCmpl(&num, 0), 0);

    bnSet(&num, 231L << 32);
    bnSet(&num2, 100);
    bnAdd(&num, &num2);
    cr_expect_eq(bnCmpl(&num, (231L << 32) + 100), 0);

    bnSet(&num, 0);
    bnSet(&num2, -1);
    bnAdd(&num, &num2);
    cr_expect_eq(bnCmpl(&num, -1), 0);

    bnSet(&num, 1L << 63);
    bnSet(&num2, (1L << 63) + 723748982738L);
    bnAdd(&num, &num2);
    cr_expect_eq(bnCmpl(&num, 1L << 63), -1);

    bnSet(&num, 1);
    bnSet(&num2, -2);
    bnAdd(&num, &num2);
    cr_expect_eq(bnCmpl(&num, -1), 0);

    free(num.words);
    free(num2.words);
}

Test(BigNumSuite, bignum_sub) {
    bignum num;
    bignum num2;
    bnInit(&num);
    bnInit(&num2);

    bnSet(&num, 17);
    bnSet(&num2, 10);

    bnSub(&num, &num2);
    cr_expect_eq(bnCmpl(&num, 7), 0);

    bnSet(&num, -100);
    bnSet(&num2, 17);
    bnSub(&num, &num2);
    cr_expect_eq(bnCmpl(&num, -117), 0);

    bnSet(&num, 45);
    bnSet(&num2, 910);
    bnSub(&num, &num2);
    cr_expect_eq(bnCmpl(&num, -865), 0);

    bnSet(&num, -(231L << 32));
    bnSet(&num2, 100);
    bnSub(&num, &num2);
    cr_expect_eq(bnCmpl(&num, -((231L << 32) + 100)), 0);
    free(num.words);
    free(num2.words);
}

Test(BigNumSuite, bignum_mul) {
    bignum num;
    bignum num2;
    bnInit(&num);
    bnInit(&num2);

    bnSet(&num, 17);
    bnSet(&num2, 100);

    bnMul(&num, &num2);
    cr_expect_eq(bnCmpl(&num, 1700), 0);
    bnSet(&num, -100);
    bnSet(&num2, 17);
    bnMul(&num, &num2);
    cr_expect_eq(bnCmpl(&num, -1700), 0);

    bnSet(&num, -100);
    bnSet(&num2, 0);
    bnMul(&num, &num2);
    cr_expect_eq(bnCmpl(&num, 0), 0);

    bnSet(&num, 0);
    bnSet(&num2, -100);
    bnMul(&num, &num2);
    cr_expect_eq(bnCmpl(&num, 0), 0);

    bnSet(&num, 231L << 32);
    bnSet(&num2, 100);
    bnMul(&num, &num2);
    cr_expect_eq(bnCmpl(&num, (231L << 32) * 100L), 0);

    bnSet(&num, 2);
    bnSet(&num2, -1);
    bnMul(&num, &num2);
    cr_expect_eq(bnCmpl(&num, -2), 0);
    free(num.words);
    free(num2.words);
}

Test(BigNumSuite, bignum_div) {

    bignum a;
    bignum b;
    bignum r;
    bnInit(&a);
    bnInit(&b);

    bnSet(&a, 1234);
    bnSet(&b, 2);
    bnDiv(&a, &b, &r);
    cr_expect_eq(bnCmpl(&a, 1234 / 2), 0);
    cr_expect_eq(bnCmpl(&r, 0), 0);

    bnSet(&a, 81);
    bnSet(&b, 9);
    bnDiv(&a, &b, &r);
    cr_expect_eq(bnCmpl(&a, 9), 0);
    cr_expect_eq(bnCmpl(&r, 0), 0);

    bnSet(&a, 100);
    bnSet(&b, 7);
    bnDiv(&a, &b, &r);
    cr_expect_eq(bnCmpl(&a, 100 / 7), 0);
    cr_expect_eq(bnCmpl(&r, 100 % 7), 0);

    bnSet(&a, 231L << 32);
    bnSet(&b, 100);
    bnDiv(&a, &b, &r);
    cr_expect_eq(bnCmpl(&a, (231L << 32) / 100), 0);
    cr_expect_eq(bnCmpl(&r, (231L << 32) % 100), 0);

    i64 na = 734895678928373L;
    i64 nb = 0xf00000000L;
    bnSet(&a, na);
    bnSet(&b, nb);
    bnDiv(&a, &b, &r);
    u64 la = a.size >= 2 ? a.words[1] : 0;
    la <<= 32;
    la |= a.size >= 1 ? a.words[0] : 0;

    u64 lr = r.size >= 2 ? r.words[1] : 0;
    la <<= 32;
    la |= r.size >= 1 ? r.words[0] : 0;
    printf("%lu <-> %lu\n", la, na / nb);
    printf("%lu <-> %lu\n", lr, na % nb);
    cr_expect_eq(bnCmpl(&a, na / nb), 0);
    cr_expect_eq(bnCmpl(&r, na % nb), 0);

    free(a.words);
    free(b.words);
    free(r.words);
}
