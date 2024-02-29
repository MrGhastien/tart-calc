#include "bignum/bignum.h"
#include <criterion/criterion.h>
#include <stdio.h>

TestSuite(BigNumSuite);

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

    bnSet(&num, (231L << 32) | 774);
    bnSet(&num2, 231L << 32);
    bnSub(&num, &num2);
    cr_expect_eq(bnCmpl(&num, 774), 0);
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

Test(BigNumSuite, bignum_euclid_div) {

    bignum a;
    bignum b;
    bignum r;
    bnInit(&a);
    bnInit(&b);
    bnInit(&r);

    bnSet(&a, 1234);
    bnSet(&b, 2);
    bnEuclidDiv(&a, &b, &r);
    cr_expect_eq(bnCmpl(&a, 1234 / 2), 0);
    cr_expect_eq(bnCmpl(&r, 0), 0);

    bnSet(&a, 81);
    bnSet(&b, 9);
    bnEuclidDiv(&a, &b, &r);
    cr_expect_eq(bnCmpl(&a, 9), 0);
    cr_expect_eq(bnCmpl(&r, 0), 0);

    bnSet(&a, 100);
    bnSet(&b, 7);
    bnEuclidDiv(&a, &b, &r);
    cr_expect_eq(bnCmpl(&a, 100 / 7), 0);
    cr_expect_eq(bnCmpl(&r, 100 % 7), 0);

    bnSet(&a, 231L << 32);
    bnSet(&b, 100);
    bnEuclidDiv(&a, &b, &r);
    cr_expect_eq(bnCmpl(&a, (231L << 32) / 100), 0);
    cr_expect_eq(bnCmpl(&r, (231L << 32) % 100), 0);

    i64 na = 734895678928373L;
    i64 nb = 0xf00000000L;
    bnSet(&a, na);
    bnSet(&b, nb);
    bnEuclidDiv(&a, &b, &r);
    cr_expect_eq(bnCmpl(&a, na / nb), 0);
    cr_expect_eq(bnCmpl(&r, na % nb), 0);

    free(a.words);
    free(b.words);
    free(r.words);
}

Test(BigNumSuite, bignum_divl) {
    bignum a;
    bnInit(&a);

    bnSet(&a, 1);
    bnDivl(&a, 2);
    cr_expect_eq(a.unitWord, 1);
    cr_expect_eq(a.words[0], 1u << 31);
    cr_expect_eq(a.size, 2);

    bnSet(&a, -1);
    bnDivl(&a, 2);
    cr_expect_eq(a.unitWord, 1);
    cr_expect_eq(a.words[0], 1u << 31);
    cr_expect_eq(a.size, 1);

    bnSet(&a, 1);
    bnDivl(&a, -2);
    cr_expect_eq(a.unitWord, 1);
    cr_expect_eq(a.words[0], 1u << 31);
    cr_expect_eq(a.size, 1);

    bnSet(&a, 1);
    bnDivl(&a, 3);
    cr_expect_eq(a.size, 10);

    free(a.words);
}

Test(BigNumSuite, bignum_div) {
    bignum a;
    bignum b;
    char* str;
    bnInit(&a);
    bnInit(&b);

    bnSet(&a, 1);
    bnSet(&b, 2);
    bnDiv(&a, &b);
    cr_expect_eq(a.unitWord, 1);
    cr_expect_eq(a.size, 2);
    cr_expect_eq(a.words[0], 1u << 31);
    bnStr(&a, &str);
    printf("%s\n", str);
    free(str);

    bnSet(&a, 1);
    bnSet(&b, 3);
    bnDiv(&a, &b);
    cr_expect_gt(a.words[a.unitWord - 1], 1u << 30);
    cr_expect_lt(a.words[a.unitWord - 1], 1u << 31);
    bnStr(&a, &str);
    printf("%s\n", str);
    free(str);

    free(a.words);
    free(b.words);
}

Test(BigNumSuite, bignum_parse) {
    const char* str = "2.5";

    bignum num;
    bnParse(str, &num);

    char* out;
    bnStr(&num, &out);
    printf("%s\n", out);
    free(out);

    free(num.words);
    
}
