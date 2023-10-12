#pragma once
#include "defines.h"

#define DEF_PRINT_ARRAY(type, format, ...)                                  \
    void darrayPrint_##type(void *array, u64 length) {                                                                 \
        typeof(type *) raw = array;                                                                                    \
        for (u64 i = 0; i < length; i++) {                                                                             \
            printf(format, __VA_ARGS__);                                                                                    \
            putc('\n');                                                                                                \
        }                                                                                                              \
    }

typedef struct darray {
    u64 capacity;
    u64 stride;
    u64 length;
    void *a;
} darray;

darray *darrayCreate(u64 size, u64 stride);
void darrayInit(darray* array, u64 size, u64 stride);

void darrayDestroy(darray *array);

/**
 * Adds an element ELEMENT to the end of the ARRAY.
 * It copies the memory pointed to by ELEMENT into the array.
 */
void _darrayAdd(darray *array, void *element);

#define darrayAdd(array, elem)                                                                                         \
    {                                                                                                                  \
        typeof(elem) holder = elem;                                                                                    \
        _darrayAdd(array, &holder);                                                                                    \
    }

/**
 * Inserts an element ELEMENT at index INDEX in the array, shifting all elements after INDEX.
 * It copies the memory pointed to by ELEMENT into the array.
 */
void _darrayInsert(darray *array, void *element, u64 index);

#define darrayInsert(array, elem, index)                                                                               \
    {                                                                                                                  \
        typeof(elem) holder = elem;                                                                                    \
        _darrayInsert(array, &holder, index);                                                                          \
    }

u64 darrayCapacity(darray *darray);
u64 darrayStride(darray *darray);
u64 darrayLength(darray *darray);

bool darrayRemove(darray *array, u64 index, void *out);
bool darrayPop(darray *array, void *out);
bool darrayPeek(darray *array, void *out);

bool darrayGet(darray *array, u64 index, void *out);

void darrayClear(darray *array);

typedef void (*destructor)(void*);

//Free the given array, and apply the given destructor to ensure no memory leaks
//occur.
void darrayDestroyDeep(darray* darray, destructor freeFunc);
