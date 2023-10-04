#include "string-builder.h"

#include <stdlib.h>

struct StringBuilder_T {
    darray* array;
};

StringBuilder* createBuilder() {
    StringBuilder* builder = malloc(sizeof *builder);
    if(builder == null)
        return null;
    builder->array = darrayCreate(4, sizeof(char));
    darrayAdd(builder->array, '\0');
    return builder;
}

void destroyBuilder(StringBuilder *builder) {
    darrayDestroy(builder->array);
    free(builder);
}

void builderAppendc(StringBuilder *builder, char c) {
    darrayInsert(builder->array, c, darrayLength(builder->array) - 1);
}

void builderAppends(StringBuilder *builder, char* str) {
    while (*str) {
        builderAppendc(builder, *str);
        str++;
    }
}

static void appendNum(StringBuilder *builder, u64 num, u64 *firstDigitIndex) {
    if (num == 0) {
        builderAppendc(builder, '0');
        return;
    }
    char digit = num % 10 + '0';
    builderAppendc(builder, digit);
    num /= 10;
    *firstDigitIndex = darrayLength(builder->array) - 2;
    while (num) {
        digit = num % 10 + '0';
        darrayInsert(builder->array, digit, *firstDigitIndex);
        num /= 10;
    }
}

void builderAppendu(StringBuilder *builder, u64 num) {
    u64 i;
    appendNum(builder, num, &i);
}

void builderAppendi(StringBuilder *builder, i64 num) {
    u64 i;
    appendNum(builder, (u64)(num < 0 ? -num : num), &i);
    if(num < 0) {
        char c = '-';
        darrayInsert(builder->array, c, i);
    }
}

void builderDeleteAt(StringBuilder *builder, u64 index) {
    if(index == builderLength(builder))
        return;
    darrayRemove(builder->array, index, null);
}

u64 builderLength(StringBuilder *builder) {
    return darrayLength(builder->array) - 1;
}

void builderReset(StringBuilder *builder) {
    darrayClear(builder->array);
    darrayAdd(builder->array, '\0');
}

const char *builderStringRef(StringBuilder *builder) {
    return (char*)builder->array->a;
}

char *builderCreateString(StringBuilder *builder) {
    u64 len = builderLength(builder);
    char* str = malloc(sizeof *str * (len + 1));
    if(str == null)
        return null;
    for (u64 i = 0; i < len; i++) {
        darrayGet(builder->array, i, str + i);
    }
    str[len] = '\0';
    return str;
}

void builderDestroy(StringBuilder *builder) { darrayDestroy(builder->array); }
