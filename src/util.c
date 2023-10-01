#include "util.h"
#include "monads.h"

/**
 * Copies size bytes from a into b.
 */
void* memcpy(void *dst, const void *src, u64 size) {
    for (u64 i = 0; i < size; i++) {
        ((u8*)dst)[i] = ((u8*)src)[i];
    }
    return dst;
}

u64 strlen(const char *str) {
    u64 len = 0;
    while (*str) {
        len++;
        str++;
    }
    return len;
}

bool streq(const char *a, const char *b) {
    while (*a && *b) {
        if(*a != *b)
            return false;
        a++;
        b++;
    }
    //Check if the last characters are both terminators
    //If not, the strings have different lengths
    return *a == *b;
}
