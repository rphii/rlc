#include "array.h"    /* keep as very first line */
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <rlc/err.h>

static inline void *rl_array_init();
static inline void *_rl_array_grow2(void *array, size_t size, size_t capacity);

#define rl_array_assert_arg(arg)   ASSERT(arg, "null pointer argument!");
#define rl_array_base(array)       ((array) - offsetof(RL_Array, data))

#define rl_array_error(msg, ...)     do { \
        ABORT("\n" "array error: " msg "\n", ##__VA_ARGS__); \
    } while(0)

typedef struct RL_Array {
    size_t length;
    size_t capacity;
    void *data;
} RL_Array;

static inline void *rl_array_init() {
    RL_Array *v = malloc(sizeof(RL_Array));
    if(!v) {
        rl_array_error("failed creating array");
    }
    memset(v, 0, sizeof(*v));
    return &v->data;
}

static inline void *_rl_array_grow2(void *array, size_t size, size_t capacity) {
    if(!array) {
        array = rl_array_init();
    }
    RL_Array *v = rl_array_base(array);
    if(capacity <= v->capacity) return array;
    if(capacity * 2 < capacity) {
        rl_array_error("invalid allocation size: %zu", capacity);
    }
    size_t require = 2;
    while(require < capacity) require *= 2;
    size_t bytes = sizeof(RL_Array) + size * require;
    if((bytes - sizeof(RL_Array)) / size != require) {
        rl_array_error("array member of %zu bytes can't allocate %zu elements", size, require);
    }
    void *temp = realloc(v, bytes);
    if(!temp) {
        rl_array_error("failed allocation of: %zu elements (%zu bytes)", require, bytes);
    }
    v = temp;
    memset((unsigned char *)&v->data + size * v->capacity, 0, size * (require - v->capacity));
    v->capacity = require;
    return &v->data;
}

void _rl_array_grow(void *array, size_t size, size_t capacity) {
    rl_array_assert_arg(array);
    void **p = array;
    *p = _rl_array_grow2(*p, size, capacity);
}

void _rl_array_resize(void *array, size_t size, size_t length) {
    rl_array_assert_arg(array);
    void **p = array;
    *p = _rl_array_grow2(*p, size, length);
    RL_Array *v = rl_array_base(*p);
    v->length = length;
}

void *_rl_array_copy(void *array, size_t size) {
    if(!array) return 0;
    void *v = rl_array_init();
    size_t len = rl_array_len(array);
    _rl_array_grow(&v, size, len);
    memcpy(v, array, size * len);
    return v;
}

void *_rl_array_addr(const void *array, size_t size, size_t index) {
    rl_array_assert_arg(array);
#if !defined(NDEBUG)
    RL_Array *v = (void *)rl_array_base(array);
    if(!(index < v->length)) {
        rl_array_error("index %zu is out of bounds %zu", index, v->length);
    }
#endif
    return (unsigned char *)array + size * index;
}

void *_rl_array_push(void *array, size_t size) {
    void **p = array; RL_Array *v = *p ? rl_array_base(*p) : 0;
    *p = _rl_array_grow2(*p, size, v ? v->length + 1 : 1);
    v = rl_array_base(*p);
    size_t index = v->length++;
    return (unsigned char *)&v->data + size * index;
}

void *_rl_array_pop(void *array, size_t size) {
    rl_array_assert_arg(array);
    RL_Array *v = rl_array_base(array);
#if !defined(NDEBUG)
    if(!v->length) {
        rl_array_error("no elements left to pop");
    }
#endif
    size_t index = --v->length;
    return array + size * index;
}

void _rl_array_free_index(RL_Array *v, size_t size, size_t index, RL_Array_Free f) {
    if(!size) return;
    rl_array_assert_arg(size);
    void *val = (unsigned char *)&v->data + size * index;
    if(!val) return;
    if(f) f(val);
}

void _rl_array_free(void *array) {
    rl_array_assert_arg(array);
    void **p = array;
    if(!*p) return;
    RL_Array *v = rl_array_base(*p);
    free(v);
    *p = 0;
}

void _rl_array_free_ext(void *array, size_t size, RL_Array_Free f) {
    rl_array_assert_arg(array);
    rl_array_assert_arg(f);
    void **p = array;
    if(!*p) return;
    RL_Array *v = rl_array_base(*p);
    for(size_t index = 0; index < v->length; ++index) {
        _rl_array_free_index(v, size, index, f);
    }
    free(v);
    *p = 0;
}

size_t _rl_array_len(const void *array) {
    if(!array) return 0;
    RL_Array *v = (RL_Array *)rl_array_base(array);
    return v->length;
}

size_t _rl_array_cap(const void *array) {
    if(!array) return 0;
    RL_Array *v = (RL_Array *)rl_array_base(array);
    return v->capacity;
}

void _rl_array_clear(void *array) {
    if(!array) return;
    RL_Array *v = rl_array_base(array);
    v->length = 0;
}

void _rl_array_clear_ext(void *array, size_t size, RL_Array_Free f) {
    if(!array) return;
    RL_Array *v = rl_array_base(array);
    for(size_t index = 0; index < v->length; ++index) {
        _rl_array_free_index(v, size, index, f);
    }
    v->length = 0;
}

