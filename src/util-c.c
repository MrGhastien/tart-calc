#include "util.h"

int ucmp(i64 a, u64 b) {
    if(a < 0)
        return -1;
    return (u64)a < b;
}
