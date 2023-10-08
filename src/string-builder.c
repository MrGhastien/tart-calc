#include "string-builder.h"

#include <stdlib.h>

StringBuilder* createBuilder() {
    return darrayCreate(4, sizeof(char));
}

void initBuilder(StringBuilder *builder) {
    darrayInit(builder, 4, sizeof(char));
    darrayAdd(builder, '\0');
}

void destroyBuilder(StringBuilder *builder) {
    darrayDestroy(builder);
    free(builder);
}

void builderAppendc(StringBuilder *builder, char c) {
    darrayInsert(builder, c, darrayLength(builder) - 1)
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
    *firstDigitIndex = darrayLength(builder) - 2;
    while (num) {
        digit = num % 10 + '0';
        darrayInsert(builder, digit, *firstDigitIndex);
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
        darrayInsert(builder, c, i);
    }
}

void builderDeleteAt(StringBuilder *builder, u64 index) {
    if(index == builderLength(builder))
        return;
    darrayRemove(builder, index, null);
}

u64 builderLength(StringBuilder *builder) {
    return darrayLength(builder) - 1;
}

void builderReset(StringBuilder *builder) {
    darrayClear(builder);
    darrayAdd(builder, '\0');
}

const char *builderStringRef(StringBuilder *builder) {
    return (char*)builder->a;
}

char *builderCreateString(StringBuilder *builder) {
    u64 len = builderLength(builder);
    char* str = malloc(sizeof *str * (len + 1));
    if(str == null)
        return null;
    for (u64 i = 0; i < len; i++) {
        darrayGet(builder, i, str + i);
    }
    str[len] = '\0';
    return str;
}

void builderDestroy(StringBuilder *builder) { darrayDestroy(builder); }
