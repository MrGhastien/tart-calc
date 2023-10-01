#pragma once
#include "defines.h"

#define DARRAY_CAPACITY 0
#define DARRAY_STRIDE 1
#define DARRAY_LENGTH 2

void* darrayCreate(u64 count, u64 stride);

void darrayDestroy(void* array);

/**
 * Adds an element ELEMENT to the end of the ARRAY.
 * It copies the memory pointed to by ELEMENT into the array.
 */
void _darrayAdd(void** array, void* element);

#define darrayAdd(array, elem) _darrayAdd(array, elem)

/**
 * Inserts an element ELEMENT at index INDEX in the array, shifting all elements after INDEX.
 * It copies the memory pointed to by ELEMENT into the array.
 */
void _darrayInsert(void** array, void* element, u64 index);

#define darrayInsert(array, elem, index) _darrayInsert(array, elem, index)

u64 _darrayGetField(void* array, u64 field);

#define darrayLength(array) _darrayGetField(array, DARRAY_LENGTH)
#define darrayCapacity(array) _darrayGetField(array, DARRAY_CAPACITY)
#define darrayStride(array) _darrayGetField(array, DARRAY_STRIDE)

bool darrayRemove(void* array, u64 index, void* out);
bool darrayPop(void* array, void *out);
bool darrayPeek(void* array, void *out);


void darrayClear(void* array);
