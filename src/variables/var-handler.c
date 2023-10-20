#include "var-handler.h"
#include "./hash.h"
#include "error.h"
#include "util.h"

#include <stdlib.h>

#define INITIAL_CAPACITY 3
#define LOAD_FACTOR_THRESHOLD 0.675
#define RESIZE_FACTOR 2
#define PERTURB_SHIFT 5

#define CAP(power) (1 << power)
#define MASK(power) ((1 << power) - 1)

typedef struct hashnode {
    char* symbol;
    u64 hash;
    double value;
} hashnode;

/**
 ** The capacity is always a power of 2. Only the power is stored.
 **/
struct varctx_t {
    hashnode* nodes;
    u64 tableCapacityPower;
    u64 nodeCount;
};

static VarCtx ctx;

static u64 tableCapacity() {
    return CAP(ctx.tableCapacityPower);
}

static double loadfactor() {
    return ((double)ctx.nodeCount) / tableCapacity();
}

void initVariables() {
    ctx.nodes = calloc(CAP(INITIAL_CAPACITY), sizeof *ctx.nodes);
    if (!ctx.nodes) {
        signalErrorNoToken(ERR_ALLOC_FAIL, NULL, -1);
        return;
    }
    ctx.tableCapacityPower = INITIAL_CAPACITY;
    ctx.nodeCount = 0;
}

void shutVariables() {
    for (u64 i = 0; i < tableCapacity(); i++) {
        if(ctx.nodes[i].symbol)
            free(ctx.nodes[i].symbol);
    }
    free(ctx.nodes);
}

static bool resizeMap(u64 newCapacityPower) {
    hashnode* newArray = malloc(sizeof *ctx.nodes * (1 << newCapacityPower));
    if (!newArray) {
        signalErrorNoToken(ERR_ALLOC_FAIL, NULL, -1);
        return false;
    }
    u64 capacity = tableCapacity();
    for (u64 i = 0; i < capacity; i++) {
        if(!ctx.nodes[i].symbol)
            continue;
        u64 hash = ctx.nodes[i].hash;
        newArray[hash & MASK(newCapacityPower)] = ctx.nodes[i];
    }
    free(ctx.nodes);
    ctx.nodes = newArray;
    ctx.tableCapacityPower = newCapacityPower;
    return true;
}

static u64 probe(const char* symbol, u64 hash, u64 index, bool* setSymbol) {
    *setSymbol = true;
    u64 perturb = hash;
    u64 i = index;
    hashnode* dst;
    while ((dst = &ctx.nodes[i])->symbol) {
        if (streq(dst->symbol, symbol)) {
            *setSymbol = false;
            return i;
        }
        i = (5 * i) + 1 + perturb;
        perturb >>= PERTURB_SHIFT;
        i &= MASK(ctx.tableCapacityPower);
    }
    return i;
}

bool setVariable(const char* symbol, double value) {
    if (loadfactor() >= LOAD_FACTOR_THRESHOLD) {
        if(!resizeMap(ctx.tableCapacityPower + 1))
            return false;
    }
    u64 h = hash(symbol);
    u64 index = h & MASK(ctx.tableCapacityPower);
    bool setSymbol;
    index = probe(symbol, h, index, &setSymbol);
    hashnode* dst = &ctx.nodes[index];

    if (setSymbol) {
        u64 len = strlen(symbol);
        dst->symbol = malloc(sizeof *dst->symbol * len + 1);
        if (!dst->symbol) {
            signalErrorNoToken(ERR_ALLOC_FAIL, NULL, 0);
            return false;
        }
        memcpy(dst->symbol, symbol, len + 1);
        ctx.nodeCount++;
    }
    dst->hash = h;
    dst->value = value;
    return true;
}

bool getVariable(const char* symbol, double* outValue) {
    u64 h = hash(symbol);
    u64 index = h & MASK(ctx.tableCapacityPower);
    bool setSymbol;
    index = probe(symbol, h, index, &setSymbol);
    hashnode* dst = &ctx.nodes[index];
    if (setSymbol) {
        return false;
    }

    *outValue = dst->value;
    return true;
}
