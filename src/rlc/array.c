#include "array.h"    /* keep as very first line */
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <rlc/err.h>

static inline void *array_init();
static inline void *_array_grow2(void *array, size_t size, size_t capacity);

#define array_assert_arg(arg)   ASSERT(arg, "null pointer argument!");
#define array_base(array)       ((array) - offsetof(Array, data))

#define array_error(msg, ...)     do { \
        ABORT("\n" "array error: " msg "\n", ##__VA_ARGS__); \
    } while(0)

typedef struct Array {
    size_t length;
    size_t capacity;
    void *data;
} Array;

static inline void *array_init() {
    Array *v = malloc(sizeof(Array));
    if(!v) {
        array_error("failed creating array");
    }
    memset(v, 0, sizeof(*v));
    return &v->data;
}

static inline void *_array_grow2(void *array, size_t size, size_t capacity) {
    if(!array) {
        array = array_init();
    }
    Array *v = array_base(array);
    if(capacity <= v->capacity) return array;
    if(capacity * 2 < capacity) {
        array_error("invalid allocation size: %zu", capacity);
    }
    size_t require = 2;
    while(require < capacity) require *= 2;
    size_t bytes = sizeof(Array) + size * require;
    if((bytes - sizeof(Array)) / size != require) {
        array_error("array member of %zu bytes can't allocate %zu elements", size, require);
    }
    void *temp = realloc(v, bytes);
    if(!temp) {
        array_error("failed allocation of: %zu elements (%zu bytes)", require, bytes);
    }
    v = temp;
    memset((void *)&v->data + size * v->capacity, 0, size * (require - v->capacity));
    v->capacity = require;
    return &v->data;
}

void _array_grow(void *array, size_t size, size_t capacity) {
    array_assert_arg(array);
    void **p = array;
    *p = _array_grow2(*p, size, capacity);
}

void _array_resize(void *array, size_t size, size_t length) {
    array_assert_arg(array);
    void **p = array;
    *p = _array_grow2(*p, size, length);
    Array *v = array_base(*p);
    v->length = length;
}

void *_array_copy(void *array, size_t size) {
    if(!array) return 0;
    void *v = array_init();
    size_t len = array_len(array);
    _array_grow(&v, size, len);
    memcpy(v, array, size * len);
    return v;
}

void *_array_addr(const void *array, size_t size, size_t index) {
    array_assert_arg(array);
#if !defined(NDEBUG)
    Array *v = (void *)array_base(array);
    if(!(index < v->length)) {
        array_error("index %zu is out of bounds %zu", index, v->length);
    }
#endif
    return (void *)array + size * index;
}

void *_array_push(void *array, size_t size) {
    void **p = array; Array *v = *p ? array_base(*p) : 0;
    *p = _array_grow2(*p, size, v ? v->length + 1 : 1);
    v = array_base(*p);
    size_t index = v->length++;
    return (void *)&v->data + size * index;
}

void *_array_pop(void *array, size_t size) {
    array_assert_arg(array);
    Array *v = array_base(array);
#if !defined(NDEBUG)
    if(!v->length) {
        array_error("no elements left to pop");
    }
#endif
    size_t index = --v->length;
    return array + size * index;
}

void _array_free_index(Array *v, size_t index, size_t size, Array_Free f) {
    if(!size) return;
    array_assert_arg(size);
    void *val = (void *)&v->data + size * index;
    if(!val) return;
    if(f) f(val);
}

void _array_free(void *array) {
    array_assert_arg(array);
    void **p = array;
    if(!*p) return;
    Array *v = array_base(*p);
    free(v);
    *p = 0;
}

void _array_free_ext(void *array, size_t size, Array_Free f) {
    array_assert_arg(array);
    array_assert_arg(f);
    void **p = array;
    if(!*p) return;
    Array *v = array_base(*p);
    for(size_t index = 0; index < v->length; ++index) {
        _array_free_index(v, index, size, f);
    }
    *p = 0;
}

size_t _array_len(const void *array) {
    if(!array) return 0;
    Array *v = (Array *)array_base(array);
    return v->length;
}

size_t _array_cap(const void *array) {
    if(!array) return 0;
    Array *v = (Array *)array_base(array);
    return v->capacity;
}

void _array_clear(void *array) {
    if(!array) return;
    Array *v = array_base(array);
    v->length = 0;
}

void _array_clear_ext(void *array, size_t size, Array_Free f) {
    if(!array) return;
    Array *v = array_base(array);
    for(size_t index = 0; index < v->length; ++index) {
        _array_free_index(v, index, size, f);
    }
    v->length = 0;
}

