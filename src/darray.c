#include "darray.h"
#include "util.h"

#include <stdlib.h>
#include <err.h>

#define FIELD_SIZE 3 * sizeof(u64)

darray* darrayCreate(u64 size, u64 stride) {
    darray* array = malloc(sizeof *array);
    darrayInit(array, size, stride);
    return array;
}

void darrayInit(darray *array, u64 size, u64 stride) {
    array->capacity = size;
    array->stride = stride;
    array->length = 0;
    array->a = malloc(size * stride);
}

void darrayDestroy(darray* array) {
    darrayEmpty(array);
    free(array);
}

void darrayEmpty(darray* array) {
    free(array->a);
}

u64 darrayCapacity(darray *array) {
    return array->capacity;
}

u64 darrayStride(darray *array) {
    return array->stride;
}

u64 darrayLength(darray *array) {
    return array->length;
}

static bool ensureCapacity(darray* array) {
    u64 capacity = darrayCapacity(array);
    u64 stride = darrayStride(array);
    u64 newCapacity = capacity << 1;

    void* ptr = realloc(array->a, newCapacity * stride);
    if(ptr == NULL)
        return false;
    array->a = ptr;
    array->capacity = newCapacity;
    return true;
}

void _darrayAdd(darray* array, void *element) {
    u64 capacity = darrayCapacity(array);
    u64 stride = darrayStride(array);
    u64 length = darrayLength(array);

    if (capacity <= length) {
        if(!ensureCapacity(array))
            return;
        capacity = darrayCapacity(array);
    }

    memcpy(array->a + length * stride, element, stride);
    array->length++;
}

void _darrayInsert(darray* array, void *element, u64 index) {
    u64 capacity = darrayCapacity(array);
    u64 stride = darrayStride(array);
    u64 length = darrayLength(array);
    if (index == length) {
        return _darrayAdd(array, element);
    }

    if (capacity == length) {
        if(!ensureCapacity(array))
            return;
        capacity = darrayCapacity(array);
    }

    void* addr = array->a;
    for (u64 i = length - 1; i > index; i--) {
        memcpy(addr + (i + 1) * stride, addr + i * stride, stride);
    }
    memcpy(addr + index * stride, element, stride);
    array->length++;
}

bool darrayRemove(darray* array, u64 index, void *out) {
    u64 stride = darrayStride(array);
    u64 length = darrayLength(array);

    if(index >= length)
        return false;

    void* address = array->a + index * stride;
    if(out != null)
        memcpy(out, address, stride);

    memcpy(address, address + stride, (length - index - 1) * stride);
    array->length--;
    return true;
}

bool darrayPop(darray* array, void *out) {
    u64 length = darrayLength(array);
    if(length == 0)
        return false;
    u64 stride = darrayStride(array);
    if (out != null) {
        void* addr = array->a;
        memcpy(out, addr + (length - 1) * stride, stride);
    }
    array->length--;
    return true;
}

bool darrayPeek(darray* array, void* out) {
    return darrayGet(array, darrayLength(array) - 1, out);
}

bool darrayGet(darray* array, u64 index, void* out) {
    u64 length = darrayLength(array);
    u64 stride = darrayStride(array);

    if (length == 0) {
        return false;
    }
    if (index >= length) {
        err(ERRCODE_IDX_OOB, "Index out of bounds.");
    }

    if (out != null) {
        void* addr = array->a;
        memcpy(out, addr + index * stride, stride);
    }
    return true;
}

void* darrayGetPtr(darray* array, u64 index) {
    if (index >= darrayLength(array)) {
        return null;
    }
    return array->a + index * array->stride;
}

void darrayClear(darray* array) {
    array->length = 0;
}

void darrayClearDeep(darray* array, destructor freeFunc) {
    for (u64 i = 0; i < array->length; i++) {
        freeFunc(darrayGetPtr(array, i));
    }
    darrayClear(array);
}

void darrayDestroyDeep(darray* array, destructor freeFunc) {
    darrayClearDeep(array, freeFunc);
    darrayDestroy(array);
}
