#include "bignum/bignum.h"
#include <criterion/criterion.h>

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
    /*
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
    */
}
