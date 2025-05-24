#ifndef ARRAY_SIMPLE_H

#include <stddef.h>

/* debug optimization {{{ */

#if defined(NDEBUG)
#define ARRAY_DEBUG_INFO
#define ARRAY_DEBUG_ARG
#define ARRAY_DEBUG_ARGS
#define ARRAY_DEBUG_DEF
#define ARRAY_DEBUG_DEFS
#define ARRAY_DEBUG_FMT
#else
#define ARRAY_DEBUG_INFO  , __FILE__, __LINE__, __func__
#define ARRAY_DEBUG_ARG   file, line, func
#define ARRAY_DEBUG_ARGS  , ARRAY_DEBUG_ARG
#define ARRAY_DEBUG_DEF   const char *file, const int line, const char *func
#define ARRAY_DEBUG_DEFS  , ARRAY_DEBUG_DEF
#define ARRAY_DEBUG_FMT   "%s:%u:%s() "
#endif

/*}}}*/

/* functions for use {{{ */

#define array_grow(array, capacity)     _array_grow(&array ARRAY_DEBUG_INFO, sizeof(*array), capacity)
#define array_resize(array, length)     _array_resize(&array ARRAY_DEBUG_INFO, sizeof(*array), length)
#define array_copy(array)               _array_copy(array ARRAY_DEBUG_INFO, sizeof(*array))
#define array_push(array, item)         (*(typeof(array))_array_push(&array ARRAY_DEBUG_INFO, sizeof(*array)) = item)
#define array_pop(array, item)          *(typeof(array))_array_pop(array ARRAY_DEBUG_INFO, sizeof(*array))
#define array_at(array, index)          *(typeof(array))_array_addr(array ARRAY_DEBUG_INFO, sizeof(*array), index)
#define array_it(array, index)          (typeof(array))_array_addr(array ARRAY_DEBUG_INFO, sizeof(*array), index)
#define array_len(array)                _array_len(array)
#define array_cap(array)                _array_cap(array)
#define array_clear(array)              _array_clear(array)
#define array_free(array)               _array_free(&array)

/*}}}*/

/* internal functions {{{ */

void _array_grow(void *array ARRAY_DEBUG_DEFS, size_t size, size_t capacity);
void _array_resize(void *array ARRAY_DEBUG_DEFS, size_t size, size_t length);
void *_array_copy(void *array ARRAY_DEBUG_DEFS, size_t size);
void *_array_push(void *array ARRAY_DEBUG_DEFS, size_t size);
void *_array_pop(void *array ARRAY_DEBUG_DEFS, size_t size);
void *_array_addr(const void *array ARRAY_DEBUG_DEFS, size_t size, size_t index);
size_t _array_len(const void *array);
size_t _array_cap(const void *array);
void _array_clear(void *array);
void _array_free(void *array);

/*}}}*/

#define ARRAY_SIMPLE_H
#endif

