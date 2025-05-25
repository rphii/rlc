#define _GNU_SOURCE

#include "str2.h"
#include "err.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define STR2_HASH_PRECOMP(str)  if(str->hash_src == __func__) return str->hash_val
#define STR2_HASH_SET(str, hash)        do { \
        str->hash_src = (void *)__func__; \
        str->hash_val = hash; \
    } while(0)
#define STR2_HASH_CLEAR(str)        do { \
        str->hash_src = 0; \
    } while(0)

struct Str2 str2_l(char *str) { /*{{{*/
    Str2 result = { .len = strlen(str), .str = str };
    return result;
} /*}}}*/

struct Str2 str2_ll(char *str, size_t len) { /*{{{*/
    Str2 result = { .len = len, .str = str };
    return result;
} /*}}}*/

Str2 str2_i0(Str2 str, size_t i0) { /*{{{*/
    Str2 result = {0};
    result.str = str2_it(str, i0);
    result.len = result.str - str2_it(str, 0);
    return result;
} /*}}}*/

Str2 str2_iE(Str2 str, size_t iE) { /*{{{*/
    Str2 result = {0};
    result.str = str.str;
    result.len = str2_it(str, iE) - str2_it(str, 0);
    return result;
} /*}}}*/

Str2 str2_i0iE(Str2 str, size_t i0, size_t iE) { /*{{{*/
    Str2 result = {0};
    result.str = str2_it(str, i0);
    result.len = str2_it(str, iE) - str2_it(str, i0);
    return result;
} /*}}}*/

Str2 str2_trim(Str2 str) {
    Str2 result = str2_triml(str2_trimr(str));
    return result;
}

Str2 str2_triml(Str2 str) {
    size_t len = str2_len(str);
    Str2 result = str2_ll(str.str, len);
    for(size_t i = 0; i < len; ++i) {
        if(!isspace(str2_at(result, 0))) break;
        ++result.str;
        --result.len;
    }
    return result;
}

Str2 str2_trimr(Str2 str) {
    size_t len = str2_len(str);
    Str2 result = str2_ll(str.str, len);
    for(size_t i = len; i > 0; --i) {
        if(!isspace(str2_at(result, result.len - 1))) break;
        --result.len;
    }
    return result;
}

Str2 str2_get_ext(Str2 str) {
    size_t len = str2_len(str);
    Str2 result = str2_ll(str.str, len);
    if(len) {
        size_t i = str2_rfind_ch(result, '.');
        if(i < len) {
            /* in case we have something like: file.dir/filename -> / is after . */
            size_t j = str2_rfind_ch(result, PLATFORM_CH_SUBDIR);
            if((j < len && j < i) || (j == len)) {
                result.str += i;
                result.len = len - i;
            }
        }
    }
    return result;
}

Str2 str2_get_noext(Str2 str) {
    size_t len = str2_len(str);
    Str2 result = str2_ll(str.str, len);
    if(len) {
        size_t i = str2_rfind_ch(result, '.');
        if(i < len) {
            /* in case we have something like: file.dir/filename -> / is after . */
            size_t j = str2_rfind_ch(result, PLATFORM_CH_SUBDIR);
            if((j < len && j < i) || (j == len)) {
                result.len = i;
            }
        }
    }
    return result;
}

Str2 str2_get_dir(Str2 str) {
    size_t len = str2_len(str);
    Str2 result = str2_ll(str.str, len);
    if(len) {
        size_t i0 = str2_rfind_ch(result, '/');
        if(i0 < len && PLATFORM_CH_SUBDIR != '/') {
            i0 = str2_rfind_ch(result, PLATFORM_CH_SUBDIR);
        }
        /*if(i0 < len) ++i0;*/
        else if(i0 >= len) i0 = 0;
        result.len = i0;
    }
    return result;
}

Str2 str2_get_nodir(Str2 str) {
    size_t len = str2_len(str);
    Str2 result = str2_ll(str.str, len);
    if(len) {
        size_t i0 = str2_rfind_ch(result, '/');
        if(i0 < len && PLATFORM_CH_SUBDIR != '/') {
            i0 = str2_rfind_ch(result, PLATFORM_CH_SUBDIR);
        }
        if(i0 < len) ++i0;
        else if(i0 >= len) i0 = 0;
        result.str += i0;
        result.len -= i0;
    }
    return result;
}

Str2 str2_get_basename(Str2 str) {
    size_t len = str2_len(str);
    Str2 result = str2_ll(str.str, len);
    if(len) {
        size_t iE = str2_rfind_ch(result, '.');
        size_t i0 = str2_rfind_ch(result, '/');
        if(i0 < len && PLATFORM_CH_SUBDIR != '/') {
            i0 = str2_rfind_ch(result, PLATFORM_CH_SUBDIR);
        }
        if(i0 < len) ++i0;
        if(iE < i0) iE = len;
        else if(i0 >= len) i0 = 0;
        result.str += i0;
        result.len = (iE - i0);
        /*TRYC(str_fmt(basename, "%.*s", (int)(iE - i0), str_iter_at(str, i0)));*/
    }
    return result;
}

bool str2_is_heap(Str2 str) { /*{{{*/
    return str.len & STR2_BIT_DYNAMIC;
} /*}}}*/

size_t str2_len(Str2 str) { /*{{{*/
#ifndef NDEBUG
    if(str2_is_heap(str)) {
        //printff("str.len %zu, array_len %zu", x.len & ~STR2_BIT_DYNAMIC, array_len(x.str));
        ASSERT((str.len & ~STR2_BIT_DYNAMIC) + 1 == array_len(str.str), "length of dynamic array has to be +1 of actual");
    }
#endif
    return str.len & ~STR2_BIT_DYNAMIC;
} /*}}}*/

size_t str2_hash(Str2 *str) { /*{{{*/
    ASSERT_ARG(str);
    STR2_HASH_PRECOMP(str);
    size_t hash = 5381;
    size_t i = 0;
    while(i < str2_len(*str)) {
        unsigned char c = str->str[i++];
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    //printff("calculated new hash: %zx [%s]", hash, str->str);
    STR2_HASH_SET(str, hash);
    return hash;
} /*}}}*/

size_t str2_hash_ci(Str2 *str) { /*{{{*/
    ASSERT_ARG(str);
    STR2_HASH_PRECOMP(str);
    size_t hash = 5381;
    size_t i = 0;
    while(i < str2_len(*str)) {
        unsigned char c = tolower(str->str[i++]);
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    //printff("calculated new hash: %zx [%s]", hash, str->str);
    STR2_HASH_SET(str, hash);
    return hash;
} /*}}}*/

int str2_cmp(Str2 a, Str2 b) { /*{{{*/
    size_t la = str2_len(a); 
    size_t lb = str2_len(b);
    if(la != lb) return -1;
    int result = memcmp(a.str, b.str, la);
    return result;
} /*}}}*/

int str2_cmp_ci(Str2 a, Str2 b) { /*{{{*/
    size_t la = str2_len(a); 
    size_t lb = str2_len(b);
    if(la != lb) return -1;
    for (size_t i = 0; i < str2_len(a); ++i) {
        int d = tolower(str2_at(a, i)) - tolower(str2_at(b, i));
        if (d != 0) return d;
    }
    return 0;
} /*}}}*/

int str2_hcmp(Str2 *a, Str2 *b) { /*{{{*/
    ASSERT_ARG(a);
    ASSERT_ARG(b);
    size_t la = str2_len(*a); 
    size_t lb = str2_len(*b);
    if(la != lb) return -1;
    size_t ha = str2_hash(a);
    size_t hb = str2_hash(b);
    if(ha != hb) return -1;
    int result = memcmp(a->str, b->str, la);
    return result;
} /*}}}*/

int str2_hcmp_ci(Str2 *a, Str2 *b) { /*{{{*/
    ASSERT_ARG(a);
    ASSERT_ARG(b);
    size_t la = str2_len(*a); 
    size_t lb = str2_len(*b);
    if(la != lb) return -1;
    size_t ha = str2_hash_ci(a);
    size_t hb = str2_hash_ci(b);
    if(ha != hb) return -1;
    for (size_t i = 0; i < str2_len(*a); ++i) {
        int d = tolower(str2_at(*a, i)) - tolower(str2_at(*b, i));
        if (d != 0) return d;
    }
    return 0;
} /*}}}*/

size_t str2_find_ch(Str2 str, char c) { /*{{{*/
    for(size_t i = 0; i < str2_len(str); ++i) {
        if(str.str[i] == c) return i;
    }
    return str2_len(str);
} /*}}}*/

size_t str2_rfind_ch(Str2 str, char c) { /*{{{*/
    for(size_t i = str2_len(str); i > 0; --i) {
        if(str.str[i - 1] == c) return i - 1;
    }
    return str2_len(str);
} /*}}}*/

char str2_at(Str2 str, size_t i) {
    ASSERT(i < str2_len(str), "out of bounds: %zu @ [0..%zu)", i, str2_len(str));
    return str.str[i];
}

char *str2_it(Str2 str, size_t i) {
    ASSERT(i <= str2_len(str), "out of bounds: %zu @ [0..%zu]", i, str2_len(str));
    return &str.str[i];
}

void str2_clear(Str2 *str) { /*{{{*/
    str->len = 0;
    STR2_HASH_CLEAR(str);
} /*}}}*/

void str2_free(Str2 *str) { /*{{{*/
    if(!str) return;
    if(!str2_is_heap(*str)) return;
    array_free(str->str);
    memset(str, 0, sizeof(*str));
} /*}}}*/

void str2_fmt(Str2 *str, char *format, ...) { /*{{{*/
    ASSERT_ARG(str);
    ASSERT_ARG(format);
    va_list va;
    va_start(va, format);
    str2_fmt_va(str, format, va);
    va_end(va);
} /*}}}*/

void str2_fmt_va(Str2 *str, const char *format, va_list va) { /*{{{*/
    ASSERT_ARG(str);
    ASSERT_ARG(format);
    va_list argp2;
    va_copy(argp2, va);
    size_t len_app = (size_t)vsnprintf(0, 0, format, argp2);
    va_end(argp2);

    if((int)len_app < 0) {
        ABORT("len_app is < 0!");
    }

    // calculate required memory
    size_t len_old = str2_len(*str);
    size_t len_new = len_old + len_app;
    char *result = str2_is_heap(*str) ? str->str : 0;
    array_resize(result, len_new + 1);
    if(!str2_is_heap(*str)) {
        memcpy(result, str->str, len_old);
    }

    // actual append
    int len_chng = vsnprintf(&result[len_old], len_app + 1, format, va);

    // check for success
    if(!(len_chng >= 0 && (size_t)len_chng <= len_app)) {
        ABORT("len_chng is < 0!");
    }

    str->len += (size_t)len_chng; // successful, change length
    str->len |= STR2_BIT_DYNAMIC;
    str->str = result;
    STR2_HASH_CLEAR(str);
} /*}}}*/

Str2 str2_copy(Str2 str) { /*{{{*/
    Str2 result = {0};
    str2_extend(&result, str);
    return result;
} /*}}}*/

void str2_extend(Str2 *str, Str2 ext) {
    ASSERT_ARG(str);
    size_t len_app = str2_len(ext);

    // calculate required memory
    size_t len_old = str2_len(*str);
    size_t len_new = len_old + len_app;
    char *result = str2_is_heap(*str) ? str->str : 0;
    array_resize(result, len_new + 1);
    if(!str2_is_heap(*str)) {
        memcpy(result, str->str, len_old);
    }

    // actual append
    memcpy(&result[len_old], ext.str, len_app);
    result[len_new] = 0;

    str->len += (size_t)len_app; // successful, change length
    ASSERT(str->len == len_new, "lengths should match: %zu/%zu", str->len, len_old);
    str->len |= STR2_BIT_DYNAMIC;
    str->str = result;
    STR2_HASH_CLEAR(str);
}

