#ifndef ARRAY_SIMPLE_H

#include <stddef.h>

typedef void (*RL_Array_Free)(void *);

#define ARRAY_FREE(T)   \
    typeof((void)(*)(T))

/* functions for use {{{ */

/* take address automatically */
#define rl_array_grow(array, capacity)     _rl_array_grow(&array, sizeof(*array), capacity)
#define rl_array_resize(array, length)     _rl_array_resize(&array, sizeof(*array), length)
#define rl_array_push(array, item)         (*(typeof(array))_rl_array_push(&array, sizeof(*array)) = item)
#define rl_array_free(array)               _rl_array_free(&array)
#define rl_array_free_ext(array, f)        _rl_array_free_ext(&array, sizeof(*array), (RL_Array_Free)(void(*)(typeof(array)))(f))

/* don't take address */
#define rl_array_copy(array)               _rl_array_copy(array, sizeof(*array))
#define rl_array_pop(array)                *(typeof(array))_rl_array_pop(array, sizeof(*array))
#define rl_array_at(array, index)          *(typeof(array))_rl_array_addr(array, sizeof(*array), index)
#define rl_array_it(array, index)          (typeof(array))_rl_array_addr(array, sizeof(*array), index)
#define rl_array_len(array)                _rl_array_len(array)
#define rl_array_cap(array)                _rl_array_cap(array)
#define rl_array_clear(array)              _rl_array_clear(array)
#define rl_array_clear_ext(array, f)       _rl_array_clear_ext(array, sizeof(*array), (RL_Array_Free)(void(*)(typeof(array)))(f))
#define rl_array_itE(array)                (array ? (typeof(array))array + _rl_array_len(array) : 0)
#define rl_array_itL(array)                (typeof(array))_rl_array_addr(array, sizeof(*array), _rl_array_len(array) - 1)
#define rl_array_atL(array)                *(typeof(array))_rl_array_addr(array, sizeof(*array), _rl_array_len(array) - 1)

/*}}}*/

/* macros {{{ */
#define rl_array_extend(array, other)      do { \
            for(typeof(array) it_array_internal = other; it_array_internal < rl_array_itE(other); ++it_array_internal) { \
                rl_array_push(array, *it_array_internal); \
            } \
        } while(0)
/*}}}*/

/* internal functions {{{ */

void _rl_array_grow(void *array, size_t size, size_t capacity);
void _rl_array_resize(void *array, size_t size, size_t length);
void *_rl_array_copy(void *array, size_t size);
void *_rl_array_push(void *array, size_t size);
void *_rl_array_pop(void *array, size_t size);
void *_rl_array_addr(const void *array, size_t size, size_t index);
size_t _rl_array_len(const void *array);
size_t _rl_array_cap(const void *array);
void _rl_array_clear(void *array);
void _rl_array_clear_ext(void *array, size_t size, RL_Array_Free f);
void _rl_array_free(void *array);
void _rl_array_free_ext(void *array, size_t size, RL_Array_Free f);

/*}}}*/

#define ARRAY_SIMPLE_H
#endif

