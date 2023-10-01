#pragma once
#include "defines.h"

void* memcpy(void* dst, const void* src, u64 bytes);

u64 strlen(const char* str);

bool streq(const char *a, const char *b);
