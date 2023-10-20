#include "defines.h"
#include "var-handler.h"

#define HASH_INITIAL_VALUE 5381

static u64 rol(u64 h, u32 offset) {
    u64 overflow = h >>= sizeof h * 8 - offset;
    return (h << offset) | overflow;
}

u64 hash(const char* str) {
    u64 h = HASH_INITIAL_VALUE;
    for (u64 i = 0; str[i]; i++) {
        h ^= str[i];
        h = rol(h, 7);
    }
    return h;
}
