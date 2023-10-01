#pragma once
#include "darray.h"

typedef struct StringBuilder_T StringBuilder;

StringBuilder* createBuilder();
void destroyBuilder(StringBuilder* builder);

void builderAppendc(StringBuilder* builder, char c);
void builderAppends(StringBuilder* builder, char* str);
void builderAppendu(StringBuilder* builder, u64 num);
void builderAppendi(StringBuilder* builder, i64 num);

void builderDeleteAt(StringBuilder* builder, u64 index);

/**
 * Allocates a new string, then copies all characters in the builder inside the allocated array.
 */
const char *builderStringRef(StringBuilder *builder);

/**
 * Allocates a new string, then copies all characters in the builder inside the allocated array.
 */
char *builderCreateString(StringBuilder *builder);

u64 builderLength(StringBuilder* builder);

void builderReset(StringBuilder *builder);
