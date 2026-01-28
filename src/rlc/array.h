#ifndef ARRAY_SIMPLE_H

#include <stddef.h>

typedef void (*Array_Free)(void *);

#define ARRAY_FREE(T)   \
    typeof((void)(*)(T))

/* functions for use {{{ */

/* take address automatically */
#define array_grow(array, capacity)     _array_grow(&array, sizeof(*array), capacity)
#define array_resize(array, length)     _array_resize(&array, sizeof(*array), length)
#define array_push(array, item)         (*(typeof(array))_array_push(&array, sizeof(*array)) = item)
#define array_free(array)               _array_free(&array)
#define array_free_ext(array, f)        _array_free_ext(&array, sizeof(*array), (Array_Free)(void(*)(typeof(array)))(f))

/* don't take address */
#define array_copy(array)               _array_copy(array, sizeof(*array))
#define array_pop(array)                *(typeof(array))_array_pop(array, sizeof(*array))
#define array_at(array, index)          *(typeof(array))_array_addr(array, sizeof(*array), index)
#define array_it(array, index)          (typeof(array))_array_addr(array, sizeof(*array), index)
#define array_len(array)                _array_len(array)
#define array_cap(array)                _array_cap(array)
#define array_clear(array)              _array_clear(array)
#define array_clear_ext(array, f)       _array_clear_ext(array, (Array_Free)(void(*)(typeof(array)))(f))
#define array_itE(array)                (array ? (typeof(array))array + _array_len(array) : 0)
#define array_itL(array)                (typeof(array))_array_addr(array, sizeof(*array), _array_len(array) - 1)
#define array_atL(array)                *(typeof(array))_array_addr(array, sizeof(*array), _array_len(array) - 1)

/*}}}*/

/* macros {{{ */
#define array_extend(array, other)      do { \
            for(typeof(array) it_array_internal = other; it_array_internal < array_itE(other); ++it_array_internal) { \
                array_push(array, *it_array_internal); \
            } \
        } while(0)
/*}}}*/

/* internal functions {{{ */

void _array_grow(void *array, size_t size, size_t capacity);
void _array_resize(void *array, size_t size, size_t length);
void *_array_copy(void *array, size_t size);
void *_array_push(void *array, size_t size);
void *_array_pop(void *array, size_t size);
void *_array_addr(const void *array, size_t size, size_t index);
size_t _array_len(const void *array);
size_t _array_cap(const void *array);
void _array_clear(void *array);
void _array_clear_ext(void *array, size_t size, Array_Free f);
void _array_free(void *array);
void _array_free_ext(void *array, size_t size, Array_Free f);

/*}}}*/

#define ARRAY_SIMPLE_H
#endif

