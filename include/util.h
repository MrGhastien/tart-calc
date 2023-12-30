#pragma once
#include "defines.h"

void* memcpy(void* dst, const void* src, u64 bytes);

u64 strlen(const char* str);

bool streq(const char *a, const char *b);

u64 maxl(u64 a, u64 b);
u32 max(u32 a, u32 b);
