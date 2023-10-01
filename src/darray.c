#include "darray.h"
#include "util.h"

#include <stdlib.h>

#define FIELD_SIZE 3 * sizeof(u64)

darray darrayCreate(u64 size, u64 stride) {
    u64* array = malloc(size * stride + FIELD_SIZE);
    array[DARRAY_CAPACITY] = size;
    array[DARRAY_STRIDE] = stride;
    array[DARRAY_LENGTH] = 0;
    return (void*)array + FIELD_SIZE;
}

void darrayDestroy(void* array) {
    free(array - FIELD_SIZE);
}

u64 _darrayGetField(void* array, u64 field) {
    u64* header = array - FIELD_SIZE;
    return header[field];
}

static u64 darraySetField(void* array, u64 field, u64 value) {
    u64* header = array - FIELD_SIZE;
    return header[field] = value;
}

static void* ensureCapacity(void* array) {
    u64 capacity = darrayCapacity(array);
    u64 stride = darrayStride(array);
    u64 newCapacity = capacity << 1;

    void* ptr = realloc(array - FIELD_SIZE, newCapacity * stride + FIELD_SIZE);
    if(ptr == NULL)
        return false;
    array = ptr + FIELD_SIZE;
    darraySetField(array, DARRAY_CAPACITY, newCapacity);
    
    return array;
}

void _darrayAdd(void** array_p, void *element) {
    void* array = *array_p;
    u64 capacity = darrayCapacity(array);
    u64 stride = darrayStride(array);
    u64 length = darrayLength(array);

    if (capacity <= length) {
        array = ensureCapacity(array);
        capacity = darrayCapacity(array);
    }

    memcpy(array + length * stride, element, stride); 
    darraySetField(array, DARRAY_LENGTH, length + 1);
    *array_p = array;
}

void _darrayInsert(void** array_p, void *element, u64 index) {
    void* array = *array_p;
    u64 capacity = darrayCapacity(array);
    u64 stride = darrayStride(array);
    u64 length = darrayLength(array);
    if (index == length) {
        return _darrayAdd(array, element);
    }

    if (capacity == length) {
        array = ensureCapacity(array);
        capacity = darrayCapacity(array);
    }

    for (u64 i = length - 1; i > index; i--) {
        memcpy(array + (i + 1) * stride, array + i * stride, stride);
    }
    memcpy(array + index * stride, element, stride);
    darraySetField(array, DARRAY_LENGTH, length + 1);
    *array_p = array;
}

bool darrayRemove(void* array, u64 index, void *out) {
    u64 stride = darrayStride(array);
    u64 length = darrayLength(array);

    if(index >= length)
        return false;

    void* address = array + index * stride;
    if(out != null)
        memcpy(out, address, stride);

    memcpy(address, address + stride, (length - index - 1) * stride);
    darraySetField(array, DARRAY_LENGTH, length - 1);
    return true;
}

bool darrayPop(void* array, void *out) {
    u64 length = darrayLength(array);
    if(length == 0)
        return false;
    u64 stride = darrayStride(array);
    if(out != null)
        memcpy(out, array + (length - 1) * stride, stride);
    darraySetField(array, DARRAY_LENGTH, length - 1);
    return true;
}

bool darrayPeek(void* array, void* out) {
    u64 length = darrayLength(array);
    u64 stride = darrayStride(array);
    if(length == 0)
        return false;
    if(out != null)
        memcpy(out, array + (length - 1) * stride, stride);
    return true;
}

void darrayClear(void* array) {
    darraySetField(array, DARRAY_LENGTH, 0);
}
