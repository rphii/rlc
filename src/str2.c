#define _GNU_SOURCE

#include "str2.h"
#include "err.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

void str2_resize(Str2 *str, size_t len) {
    ASSERT_ARG(str);
    if(!str2_is_dynamic(*str)) ABORT("attempting to resize constant string");
    char *result = str2_is_heap(*str) ? str->str : 0;
    array_resize(result, len + 1);
    if(!str2_is_heap(*str)) {
        memcpy(result, str->str, str2_len(*str));
    }
    str->str = result;
    str->str[len] = 0;
    str->len = len;
    str->is_dynamic = true;
    str->is_heap = true;
}

void str2_zero(Str2 *str) {
    memset(str, 0, sizeof(*str));
}

#define STR2_HASH_PRECOMP(str)  if(str->hash_src == __func__) return str->hash_val
#define STR2_HASH_SET(str, hash)        do { \
        str->hash_src = (void *)__func__; \
        str->hash_val = hash; \
    } while(0)
#define STR2_HASH_CLEAR(str)        do { \
        str->hash_src = 0; \
    } while(0)

Str2 str2_dyn(Str2C str) { /*{{{*/
    str.is_dynamic = true;
    //str.len |= STR2_BIT_DYNAMIC;
    return str;
} /*}}}*/

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
    result.len = str2_len(str) + str.str - result.str;
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

Str2 str2_trim(Str2 str) { /*{{{*/
    Str2 result = str2_triml(str2_trimr(str));
    return result;
} /*}}}*/

Str2 str2_triml(Str2 str) { /*{{{*/
    size_t len = str2_len(str);
    Str2 result = str2_ll(str.str, len);
    for(size_t i = 0; i < len; ++i) {
        if(!isspace(str2_at(result, 0))) break;
        ++result.str;
        --result.len;
    }
    return result;
} /*}}}*/

Str2 str2_trimr(Str2 str) { /*{{{*/
    size_t len = str2_len(str);
    Str2 result = str2_ll(str.str, len);
    for(size_t i = len; i > 0; --i) {
        if(!isspace(str2_at(result, str2_len(result) - 1))) break;
        --result.len;
    }
    return result;
} /*}}}*/

Str2 str2_triml_nof(Str2 str) { /*{{{*/
    size_t len = str2_len(str);
    size_t i0 = 0;
    Str2 result = str2_ll(str.str, len);
    size_t m, n = str2_find_f(str2_i0(result, 0), &m);
    //if(m + 1 < str2_len(result)
    //printff("m %zu, n %zu",m,n);
    for(size_t i = 0; i < len; ++i) {
        //if(i >= n) {
        //    if(str2_len(result))
        //}
        //if(i >= m) n = str2_find_f(str2_i0(result, 0), &m);
        if(!isspace(str2_at(result, i))) break;
        i0 ++;
    }
    return str2_i0(result, i0);
} /*}}}*/

Str2 str2_ensure_dir(Str2 str) { /*{{{*/
    // !!! str = str2_trim(str);
    size_t len = str2_len(str);
    Str2 result = str2_ll(str.str, len);
    if(len > 1) {
        size_t nch = str2_rfind_nch(str, PLATFORM_CH_SUBDIR);
        if(!nch && len) ++nch;
        else if(nch < len && str2_at(str, nch) != PLATFORM_CH_SUBDIR) ++nch;
        result.len = nch;
        //if(str2_at(result, str2_len(result) - 1) == PLATFORM_CH_SUBDIR) {
        //    --result.len;
        //}
    }
    return result;
} /*}}}*/

Str2 str2_get_ext(Str2 str) { /*{{{*/
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
} /*}}}*/

Str2 str2_get_noext(Str2 str) { /*{{{*/
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
} /*}}}*/

Str2 str2_get_dir(Str2 str) { /*{{{*/
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
} /*}}}*/

Str2 str2_get_nodir(Str2 str) { /*{{{*/
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
} /*}}}*/

Str2 str2_get_basename(Str2 str) { /*{{{*/
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
} /*}}}*/

void str2_as_cstr(Str2 str, char *out, size_t len) { /*{{{*/
    ASSERT_ARG(out);
    ASSERT_ARG(len);
    size_t l = str2_len(str);
    size_t n = l < len - 1 ? l : len - 1;
    //printff("copying n: %zu [%s] %p -> %p", n, str.str, str.str, out);
    memcpy(out, str.str, n);
    out[n] = 0;
} /*}}}*/

int str2_as_bool(Str2 str, bool *out) { /*{{{*/
    Str2 in = str2_ll(str.str, str2_len(str));
    Str2 val_true[4] = {
        str2("true"),
        str2("yes"),
        str2("y"),
        str2("enable"),
    };
    Str2 val_false[4] = {
        str2("false"),
        str2("no"),
        str2("n"),
        str2("disable"),
    };
    bool expand_pool = true;
    bool result = false;
    for(size_t i = 0; i < 4; ++i) {
        if(i && !expand_pool) return -1;
        if(!str2_cmp(val_true[i], in)) { result = true; break; }
        if(!str2_cmp(val_false[i], in)) { result = false; break; }
        if(i + 1 >= 4 && expand_pool) return -1;
    }
    if(out) *out = result;
    return 0;
} /*}}}*/

int str2_as_int(Str2 str, int *out, int base) { /*{{{*/
    if(!str2_len(str)) return -1;
    char *endptr;
    char temp[32] = {0};
    str2_as_cstr(str, temp, 32);
    size_t result = strtol(temp, &endptr, base);
    if(endptr - temp != str2_len(str)) return -1;
    if(out) *out = result;
    return 0;
} /*}}}*/

int str2_as_size(Str2 str, size_t *out, int base) { /*{{{*/
    if(!str2_len(str)) return -1;
    char *endptr;
    char temp[32] = {0};
    str2_as_cstr(str, temp, 32);
    size_t result = strtoull(temp, &endptr, base);
    if(endptr - temp != str2_len(str)) return -1;
    if(out) *out = result;
    return 0;
} /*}}}*/

int str2_as_ssize(Str2 str, ssize_t *out, int base) { /*{{{*/
    if(!str2_len(str)) return -1;
    char *endptr;
    char temp[32] = {0};
    str2_as_cstr(str, temp, 32);
    size_t result = strtoll(temp, &endptr, base);
    if(endptr - temp != str2_len(str)) return -1;
    if(out) *out = result;
    return 0;
} /*}}}*/

int str2_as_float(Str2 str, float *out) { /*{{{*/
    char *endptr;
    char temp[32] = {0};
    str2_as_cstr(str, temp, 32);
    float result = strtof(temp, &endptr);
    if(endptr - temp != str2_len(str)) return -1;
    if(out) *out = result;
    return 0;
} /*}}}*/

int str2_as_double(Str2 str, double *out) { /*{{{*/
    char *endptr;
    char temp[32] = {0};
    str2_as_cstr(str, temp, 32);
    double result = strtod(temp, &endptr);
    if(endptr - temp != str2_len(str)) return -1;
    if(out) *out = result;
    return 0;
} /*}}}*/

int str2_as_color(Str2 str, Color *out) { /*{{{*/
    Color result = {0};
    size_t len = str2_len(str);
    size_t hex = 0;
    double dbl = 0;
    bool valid = false;
    if(len >= 7 && str2_at(str, 0) == '#') {
        if(str2_len(str) == 9) {
            if(str2_as_size(str2_i0(str, 1), &hex, 16)) return -1;
            result.r = (uint8_t)(hex >> 24);
            result.g = (uint8_t)(hex >> 16);
            result.b = (uint8_t)(hex >> 8);
            result.a = (uint8_t)(hex >> 0);
            valid = true;
        } else if(str2_len(str) == 7) {
            if(str2_as_size(str2_i0(str, 1), &hex, 16)) return -1;
            result.r = (uint8_t)(hex >> 16);
            result.g = (uint8_t)(hex >> 8);
            result.b = (uint8_t)(hex >> 0);
            valid = true;
        }
        else {
            valid = false;
        }
    } else if(len >= 5 && !str2_cmp0(str, str2("rgb"))) {
        if(str2_at(str, 3) == 'a') {
            Str2 triple = str2_i0(str, 4);
            triple = str2_i0(triple, str2_find_nws(triple));
            if(str2_len(triple) && str2_at(triple, 0) == '(') {
                size_t iE = str2_pair_ch(triple, ')');
                if(iE > str2_len(triple) - 1) return -1;
                triple = str2_trim(str2_i0iE(triple, 1, iE));
                size_t nsep = str2_count_ch(triple, ',');
                if(!nsep && str2_len(triple) == 8) {
                    if(!str2_as_size(triple, &hex, 16)) {
                        result.r = (uint8_t)(hex >> 24);
                        result.g = (uint8_t)(hex >> 16);
                        result.b = (uint8_t)(hex >> 8);
                        result.a = (uint8_t)(hex >> 0);
                        valid = true;
                    }
                } else if(nsep == 3) {
                    size_t i = 0;
                    for(Str2 splice = {0}; str2_splice(triple, &splice, ','); ++i) {
                        if(!splice.str) continue;
                        Str2 snum = str2_trim(splice);
                        if(i < 3) {
                            if(str2_as_size(snum, &hex, 10)) return -1;
                            if(i == 0) result.r = hex;
                            if(i == 1) result.g = hex;
                            if(i == 2) result.b = hex;
                        } else {
                            if(str2_as_double(snum, &dbl)) return -1;
                            if(i == 3) result.a = (uint8_t)round(0xFF*dbl);
                        }
                    }
                    valid = true;
                }
            }
        } else {
            Str2 triple = str2_i0(str, 3);
            triple = str2_i0(triple, str2_find_nws(triple));
            if(str2_len(triple) && str2_at(triple, 0) == '(') {
                size_t iE = str2_pair_ch(triple, ')');
                if(iE > str2_len(triple) - 1) return -1;
                triple = str2_trim(str2_i0iE(triple, 1, iE));
                size_t nsep = str2_count_ch(triple, ',');
                result.a = 0xFF;
                if(!nsep && str2_len(triple) == 6) {
                    if(!str2_as_size(triple, &hex, 16)) {
                        result.r = (uint8_t)(hex >> 16);
                        result.g = (uint8_t)(hex >> 8);
                        result.b = (uint8_t)(hex >> 0);
                        valid = true;
                    }
                } else if(nsep == 2) {
                    size_t i = 0;
                    for(Str2 splice = {0}; str2_splice(triple, &splice, ','); ++i) {
                        if(!splice.str) continue;
                        Str2 snum = str2_trim(splice);
                        if(str2_as_size(snum, &hex, 10)) return -1;
                        if(i == 0) result.r = hex;
                        if(i == 1) result.g = hex;
                        if(i == 2) result.b = hex;
                    }
                    valid = true;
                }
            }
        }
    } else {
        return -1;
    }
    if(valid && out) *out = result;
    return !valid;
} /*}}}*/

bool str2_is_heap(Str2 str) { /*{{{*/
    return str.is_heap;
} /*}}}*/

bool str2_is_dynamic(Str2 str) { /*{{{*/
    //printf("probing for dyn [%.*s] %p len %zx", STR2_F(str), str.str, str.len);
    if(!str.str || str.is_dynamic) {
        //printff(F(" YES", FG_GN));
        return true;
    }
    //printff(F(" NO", FG_RD));
    return false;
} /*}}}*/

size_t str2_len(Str2 str) { /*{{{*/
#if !defined(NDEBUG) && 0
    if(str2_is_heap(str)) {
        //printff("str.len %zu, array_len %zu", x.len & ~STR2_BIT_HEAP, array_len(x.str));
        ASSERT(str.is_heap ? (str.len + 1 == array_len(str.str)) : 1, "length of dynamic array has to be +1 of actual");
    }
#endif
    return str.len; // & ~STR2_BIT_MASK;
} /*}}}*/

size_t str2_len_nof(Str2 str) { /*{{{*/
    size_t len = str2_len(str), n = 0, m = 0;
    Str2 snip = str2_ll(str.str, len);
    Str2 pat = str2("\033[");
    for(;;) {
        snip = str2_i0(snip, m);
        n = str2_find_substr(snip, pat, false);
        if(n >= str2_len(snip)) break;
        snip = str2_i0(snip, n + str2_len(pat));
        m = str2_find_ch(snip, 'm');
        len -= (m + str2_len(pat));
        if(m++ >= str2_len(snip)) break;
        len -= (bool)(m);
    }
    return len;
} /*}}}*/

size_t str2_dhash(Str2 str) { /*{{{*/
    //STR2_HASH_PRECOMP(str);
    size_t hash = 5381;
    size_t i = 0;
    while(i < str2_len(str)) {
        unsigned char c = str.str[i++];
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    //printff("calculated new hash: %zx [%s]", hash, str->str);
    //STR2_HASH_SET(str, hash);
    return hash;
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

int str2_cmp0(Str2 a, Str2 b) { /*{{{*/
    size_t la = str2_len(a); 
    size_t lb = str2_len(b);
    if(la < lb) return -1;
    int result = memcmp(a.str, b.str, lb);
    return result;
} /*}}}*/

int str2_cmpE(Str2 a, Str2 b) { /*{{{*/
    size_t la = str2_len(a); 
    size_t lb = str2_len(b);
    if(la < lb) return -1;
    int result = memcmp(a.str + la - lb, b.str, lb);
    return result;
} /*}}}*/

int str2_cmp_ci(Str2 a, Str2 b) { /*{{{*/
    size_t la = str2_len(a); 
    size_t lb = str2_len(b);
    if(la != lb) return -1;
    for (size_t i = 0; i < la; ++i) {
        int d = tolower(str2_at(a, i)) - tolower(str2_at(b, i));
        if (d != 0) return d;
    }
    return 0;
} /*}}}*/

int str2_cmp0_ci(Str2 a, Str2 b) { /*{{{*/
    size_t la = str2_len(a); 
    size_t lb = str2_len(b);
    if(la < lb) return -1;
    for (size_t i = 0; i < lb; ++i) {
        int d = tolower(str2_at(a, i)) - tolower(str2_at(b, i));
        if (d != 0) return d;
    }
    return 0;
} /*}}}*/

int str2_cmpE_ci(Str2 a, Str2 b) { /*{{{*/
    size_t la = str2_len(a); 
    size_t lb = str2_len(b);
    if(la < lb) return -1;
    for (size_t i = 0; i < lb; ++i) {
        int d = tolower(str2_at(a, i + la - lb)) - tolower(str2_at(b, i));
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
    for (size_t i = 0; i < la; ++i) {
        int d = tolower(str2_at(*a, i)) - tolower(str2_at(*b, i));
        if (d != 0) return d;
    }
    return 0;
} /*}}}*/

size_t str2_find_f(Str2 str, size_t *out_iE) { /*{{{*/
    size_t i0 = str2_find_substr(str, str2(FS_BEG), false);
    if(out_iE) {
        Str2 s = str2_i0(str, i0);
        //printf(" find m: [");
        //str2_printraw(s);
        //printff("]");
        size_t iE = str2_find_ch(s, 'm');
        if(iE < str2_len(s)) ++iE;
        *out_iE = i0 + iE;
    }
    return i0;
} /*}}}*/

size_t str2_find_ch(Str2 str, char c) { /*{{{*/
#if 1
    size_t len = str2_len(str);
    char *s = memchr(str.str, c, len);
    if(!s) return len;
    return s - str.str;
#else
    for(size_t i = 0; i < str2_len(str); ++i) {
        if(str.str[i] == c) return i;
    }
    return str2_len(str);
#endif
} /*}}}*/

size_t str2_find_nch(Str2 str, char c) { /*{{{*/
    for(size_t i = 0; i < str2_len(str); ++i) {
        if(str.str[i] != c) return i;
    }
    return str2_len(str);
} /*}}}*/

size_t str2_find_ws(Str2 str) { /*{{{*/
    for(size_t i = 0; i < str2_len(str); ++i) {
        if(isspace(str.str[i])) return i;
    }
    return str2_len(str);
} /*}}}*/

size_t str2_find_nws(Str2 str) { /*{{{*/
    for(size_t i = 0; i < str2_len(str); ++i) {
        if(!isspace(str.str[i])) return i;
    }
    return str2_len(str);
} /*}}}*/

size_t str2_find_any(Str2 str, Str2 any) { /*{{{*/
    size_t len = str2_len(str);
    size_t len2 = str2_len(any);
    for(size_t i = 0; i < len; ++i) {
        char c = str.str[i];
        if(memchr(any.str, c, len2)) {
            return i;
        }
    }
    return len;
} /*}}}*/

size_t str2_find_nany(Str2 str, Str2 any) { /*{{{*/
    size_t len = str2_len(str);
    size_t len2 = str2_len(any);
    for(size_t i = 0; i < len; ++i) {
        char c = str.str[i];
        if(!memchr(any.str, c, len2)) {
            return i;
        }
    }
    return len;
} /*}}}*/

size_t str2_find_substr(Str2 str, Str2 sub, bool ignorecase) { /*{{{*/
    /* basic checks */
    if(!str2_len(sub)) return 0;
    if(str2_len(sub) > str2_len(str)) {
        return str2_len(str);
    }
    /* store original indices */
    Str2 ref = str;
    /* check for substring */
    size_t i = 0;
    while(str2_len(sub) <= str2_len(ref)) {
        size_t overlap = str2_count_overlap(ref, sub, ignorecase);
        if(overlap == str2_len(sub)) {
            return i;
        } else {
            i += overlap + 1;
            ref.str += overlap + 1;
            ref.len -= overlap + 1;
        }
    }
    /* restore original */
    return str2_len(str);
} /*}}}*/

size_t str2_rfind_f(Str2 str, size_t *out_iE) { /*{{{*/
    size_t i0 = str2_rfind_substr(str, str2(FS_BEG), false);
    if(out_iE) {
        Str2 s = str2_i0(str, i0);
        size_t iE = str2_rfind_ch(s, 'm');
        if(iE < str2_len(s)) ++iE;
        *out_iE = i0 + iE;
    }
    return i0;
} /*}}}*/

size_t str2_rfind_f0(Str2 str, Str2C *fmt) { /*{{{*/
    size_t i0 = str2_rfind_substr(str, str2(FS_BEG), false);
    Str2 s = str2_i0(str, i0);
    size_t iE = str2_find_ch(s, 'm');
    if(iE < str2_len(s)) ++iE;
    s = str2_iE(s, iE);
    //printf("<FMT:%.*s>",STR2_F(s));
    if(!str2_cmpE(s, str2("[0m"))) str2_clear(&s);
    if(fmt) *fmt = s;
    return i0;
} /*}}}*/

size_t str2_rfind_ch(Str2 str, char c) { /*{{{*/
#if 1
    size_t len = str2_len(str);
    char *s = memrchr(str.str, c, len);
    if(!s) return len;
    return s - str.str;
#else
    for(size_t i = str2_len(str); i > 0; --i) {
        if(str.str[i - 1] == c) return i - 1;
    }
    return str2_len(str);
#endif
} /*}}}*/

size_t str2_rfind_nch(Str2 str, char c) { /*{{{*/
    for(size_t i = str2_len(str); i > 0; --i) {
        if(str.str[i - 1] != c) return i - 1;
    }
    return 0;
} /*}}}*/

size_t str2_rfind_ws(Str2 str) { /*{{{*/
    for(size_t i = str2_len(str); i > 0; --i) {
        if(isspace(str.str[i - 1])) return i - 1;
    }
    return str2_len(str);
} /*}}}*/

size_t str2_rfind_nws(Str2 str) { /*{{{*/
    for(size_t i = str2_len(str); i > 0; --i) {
        if(!isspace(str.str[i - 1])) return i - 1;
    }
    return str2_len(str);
} /*}}}*/

size_t str2_rfind_any(Str2 str, Str2 any) { /*{{{*/
    size_t len = str2_len(str);
    size_t len2 = str2_len(any);
    for(size_t i = len; i > 0; --i) {
        char c = str.str[i - 1];
        if(memchr(any.str, c, len2)) {
            return i - 1;
        }
    }
    return len;
} /*}}}*/

size_t str2_rfind_nany(Str2 str, Str2 any) { /*{{{*/
    size_t len = str2_len(str);
    size_t len2 = str2_len(any);
    for(size_t i = len; i > 0; --i) {
        char c = str.str[i - 1];
        if(!memchr(any.str, c, len2)) {
            return i - 1;
        }
    }
    return len;
} /*}}}*/

size_t str2_rfind_substr(Str2 str, Str2 substr, bool ignorecase) { /*{{{*/
    /* basic checks */
    size_t n = str2_len(substr);
    size_t m = str2_len(str);
    if(!n) return 0;
    if(n > m) {
        return m;
    }
    const char *s = substr.str;
    for(size_t i = m - n + 1; i > 0; --i) {
        const char *t = str2_it(str, i - 1);
        if(!memcmp(s, t, n)) return i - 1;
    }
    return m;
} /*}}}*/

size_t str2_pair_ch(Str2 str, char c1) { /*{{{*/
    size_t len = str2_len(str);
    if(!len) return len;
    size_t level = 1;
    char c0 = str2_at(str, 0);
    for(size_t i = 1; i < str2_len(str); ++i) {
        char c = str2_at(str, i);
        if(c == c0) level++;
        else if(c == c1) level--;
        if(level <= 0) return i;
    }
    return len;
} /*}}}*/

size_t str2_splice(Str2 to_splice, Str2 *prev, char sep) { /*{{{*/
    ASSERT_ARG(prev);
    size_t len = str2_len(to_splice);
    Str2 result = str2_ll(to_splice.str, len);
    if(prev && prev->str) {
        size_t from = prev->str - to_splice.str + str2_len(*prev);
        Str2 search = str2_i0(to_splice, from);
        size_t offset = str2_find_ch(search, sep) + from;
        result.str += offset;
        result.len -= offset;
        if(result.str - to_splice.str < str2_len(to_splice)) {
            ++result.str;
            --result.len;
        }
    }
    result.len = str2_find_ch(result, sep);
    *prev = result;
    return result.str - to_splice.str - len;
} /*}}}*/

size_t str2_index_nof(Str2 str, size_t index) {
    size_t len = str2_len(str);
    size_t len_nof = 0;
    size_t n = 0, m = 0, i = 0;
    Str2 snip = str;
    if(!index) return 0;
    for(;;) {
        snip = str2_i0(str, i);
        n = str2_find_f(snip, &m);
        if(len_nof + n >= index) {
            //printff("DONE %zu + %zu - %zu", i, index, len_nof);
            return i + (index - len_nof);
        }
        len_nof += n;
        i += m;
        if(i >= len) break;
    }
    return str2_len(str);
}

char str2_at(Str2 str, size_t i) { /*{{{*/
    ASSERT(i < str2_len(str), "out of bounds: %zu @ [0..%zu)", i, str2_len(str));
    return str.str[i];
} /*}}}*/

char *str2_it(Str2 str, size_t i) { /*{{{*/
    ASSERT(i <= str2_len(str), "out of bounds: %zu @ [0..%zu]", i, str2_len(str));
    return &str.str[i];
} /*}}}*/

size_t str2_count_overlap(Str2 a, Str2 b, bool ignorecase) { /*{{{*/
    size_t overlap = 0;
    size_t len = str2_len(a) > str2_len(b) ? str2_len(b) : str2_len(a);
    if(!ignorecase) {
        for(size_t i = 0; i < len; ++i) {
            char ca = str2_at(a, i);
            char cb = str2_at(b, i);
            if(ca == cb) ++overlap;
            else break;
        }
    } else {
        for(size_t i = 0; i < len; ++i) {
            int ca = tolower(str2_at(a, i));
            int cb = tolower(str2_at(b, i));
            if(ca == cb) ++overlap;
            else break;
        }
    }
    return overlap;
} /*}}}*/

size_t str2_count_ch(Str2 str, char c) { /*{{{*/
    size_t result = 0;
    size_t len = str2_len(str);
    for(size_t i = 0; i < len; ++i) {
        if(str.str[i] == c) ++result;
    }
    return result;
} /*}}}*/

size_t str2_count_nch(Str2 str, char c) { /*{{{*/
    size_t result = 0;
    size_t len = str2_len(str);
    for(size_t i = 0; i < len; ++i) {
        if(str.str[i] != c) ++result;
    }
    return result;
} /*}}}*/

size_t str2_count_any(Str2 str, Str2 any) { /*{{{*/
    size_t result = 0;
    size_t len = str2_len(str);
    size_t len2 = str2_len(any);
    for(size_t i = 0; i < len; ++i) {
        if(memchr(any.str, str.str[i], len2)) ++result;
    }
    return result;
} /*}}}*/

size_t str2_count_nany(Str2 str, Str2 any) { /*{{{*/
    size_t result = 0;
    size_t len = str2_len(str);
    size_t len2 = str2_len(any);
    for(size_t i = 0; i < len; ++i) {
        if(!memchr(any.str, str.str[i], len2)) ++result;
    }
    return result;
} /*}}}*/

void str2_clear(Str2 *str) { /*{{{*/
    ASSERT_ARG(str);
    str->len = 0;
    STR2_HASH_CLEAR(str);
} /*}}}*/

void str2_free(Str2 *str) { /*{{{*/
    if(!str) return;
    //printf("free %p [%.*s]\n", str, STR2_F(*str));
    if(str2_is_heap(*str)) {
        array_free(str->str);
    }
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
    if(!str2_is_dynamic(*str)) ABORT("attempting to format constant string");
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
    str2_resize(str, len_new);

    // actual append
    int len_chng = vsnprintf(&str->str[len_old], len_app + 1, format, va);

    // check for success
    if(!(len_chng >= 0 && (size_t)len_chng <= len_app)) {
        ABORT("len_chng is < 0!");
    }

    STR2_HASH_CLEAR(str);
} /*}}}*/

void str2_fmt_fgbg(Str2 *out, const Str2 text, Color fg, Color bg, bool bold, bool italic, bool underline) { /*{{{*/
    ASSERT_ARG(out);
    if(!str2_is_dynamic(*out)) ABORT("attempting to format constant string");
    bool do_fmt = ((fg.rgba || bg.rgba || bold || italic || underline));
    if(!do_fmt) {
        str2_fmt(out, "%.*s", STR2_F(text));
        return;
    }
    char fmt[64] = {0}; /* theoretically 52 would be enough? */
    int len = sizeof(fmt)/sizeof(*fmt);
    int offs = 0;
    offs += snprintf(fmt, len, "%s", FS_BEG);
    if(fg.rgba) offs += snprintf(fmt + offs, len - offs, "%s", FS_FG3);
    if(bg.rgba) offs += snprintf(fmt + offs, len - offs, "%s", FS_BG3);
    if(bold) offs += snprintf(fmt + offs, len - offs, "%s", BOLD);
    if(italic) offs += snprintf(fmt + offs, len - offs, "%s", IT);
    if(underline) offs += snprintf(fmt + offs, len - offs, "%s", UL);
    snprintf(fmt + offs, len - offs, "%s", FS_END);
    if(fg.rgba && bg.rgba) { str2_fmt(out, fmt, fg.r, fg.g, fg.b, bg.r, bg.g, bg.b, STR2_F(text)); }
    else if(fg.rgba) {       str2_fmt(out, fmt, fg.r, fg.g, fg.b, STR2_F(text)); }
    else if(bg.rgba) {       str2_fmt(out, fmt, bg.r, bg.g, bg.b, STR2_F(text)); }
    else {                   str2_fmt(out, fmt, STR2_F(text)); }
} /*}}}*/

void str2_fmt_fgbga(Str2 *out, const Str2 text, Color fg, Color bg, bool bold, bool italic, bool underline) { /*{{{*/
    ASSERT_ARG(out);
    if(!str2_is_dynamic(*out)) ABORT("attempting to format constant string");
    bool do_fmt = ((fg.rgba || bg.rgba || bold || italic || underline));
    if(!do_fmt) {
        str2_fmt(out, "%.*s", STR2_F(text));
        return;
    }
    char fmt[64] = {0}; /* theoretically 52 would be enough? */
    int len = sizeof(fmt)/sizeof(*fmt);
    int offs = 0;
    offs += snprintf(fmt, len, "%s", FS_BEG);
    if(fg.rgba) offs += snprintf(fmt + offs, len - offs, "%s", FS_FG3);
    if(bg.rgba) offs += snprintf(fmt + offs, len - offs, "%s", FS_BG3);
    if(bold) offs += snprintf(fmt + offs, len - offs, "%s", BOLD);
    if(italic) offs += snprintf(fmt + offs, len - offs, "%s", IT);
    if(underline) offs += snprintf(fmt + offs, len - offs, "%s", UL);
    snprintf(fmt + offs, len - offs, "%s", FS_END);
    if(fg.rgba && bg.rgba) { str2_fmt(out, fmt, (uint8_t)roundf(((float)fg.r*fg.a)/255.0f), (uint8_t)roundf(((float)fg.g*fg.a)/255.0f), (uint8_t)roundf(((float)fg.b*fg.a)/255.0f), (uint8_t)roundf(((float)bg.r*bg.a)/255.0f), (uint8_t)roundf(((float)bg.g*bg.a)/255.0f), (uint8_t)roundf(((float)bg.b*bg.a)/255.0f), STR2_F(text)); }
    else if(fg.rgba) {       str2_fmt(out, fmt, (uint8_t)roundf(((float)fg.r*fg.a)/255.0f), (uint8_t)roundf(((float)fg.g*fg.a)/255.0f), (uint8_t)roundf(((float)fg.b*fg.a)/255.0f), STR2_F(text)); }
    else if(bg.rgba) {       str2_fmt(out, fmt, (uint8_t)roundf(((float)bg.r*bg.a)/255.0f), (uint8_t)roundf(((float)bg.g*bg.a)/255.0f), (uint8_t)roundf(((float)bg.b*bg.a)/255.0f), STR2_F(text)); }
    else {                   str2_fmt(out, fmt, STR2_F(text)); }
} /*}}}*/

void str2_input(Str2 *str) { /*{{{*/
    ASSERT_ARG(str);
    if(!str2_is_dynamic(*str)) ABORT("attempting to input constant string");
    int c = 0;
    while((c = getchar()) != '\n' && c != EOF) {
        str2_push(str, c);
    }
    if(!str2_len(*str) && (!c || c == EOF || c == '\n')) {
        //THROW("an error"); /* TODO describe this error. is this even an error? */
    }
    fflush(stdin);
} /*}}}*/

Str2 str2_copy(Str2 str) { /*{{{*/
    Str2 result = {0};
    str2_extend(&result, str);
    return result;
} /*}}}*/

void str2_push(Str2 *str, char c) { /*{{{*/
    ASSERT_ARG(str);
    if(!str2_is_dynamic(*str)) ABORT("attempting to push constant string");
    size_t len = str2_len(*str);
    str2_resize(str, len + 1);
    str->str[len] = c;
} /*}}}*/

void str2_extend(Str2 *str, Str2 extend) { /*{{{*/
    ASSERT_ARG(str);
    if(!str2_is_dynamic(*str)) ABORT("attempting to extend constant string");
    size_t len_app = str2_len(extend);

    // calculate required memory
    size_t len_old = str2_len(*str);
    size_t len_new = len_old + len_app;
    str2_resize(str, len_new);

    // actual append
    memcpy(&str->str[len_old], extend.str, len_app);

    STR2_HASH_CLEAR(str);
} /*}}}*/

size_t str2_writefunc(void *ptr, size_t size, size_t nmemb, Str2 *str) { /*{{{*/
    str2_fmt(str, "%.*s", size * nmemb, ptr);
    return size * nmemb;
} /*}}}*/

void vstr2_free_set(VStr2 *vstr) { /*{{{*/
    ASSERT_ARG(vstr);
    array_free_set(*vstr, Str2, (ArrayFree)str2_free);
} /*}}}*/

void str2_print(Str2 str) { /*{{{*/
    printf("%.*s", STR2_F(str));
} /*}}}*/

void str2_println(Str2 str) { /*{{{*/
    printf("%.*s\n", STR2_F(str));
} /*}}}*/

void str2_printal(Str2 str, Str2Print *p, size_t i0, size_t iE) {
    ASSERT_ARG(p);
    if(iE <= i0) return;
    size_t len = str2_len(str);
    size_t w = iE - i0;
    for(size_t j0 = 0; j0 < len; ) {
        if(p->nl_pending) {
            p->nl_pending = false;
            printf("\n");
        }
        //if(jE > len) jE = len;
        Str2 bufws = str2_triml_nof(str2_i0(str, j0));
        if(!str2_len(bufws)) break;
        //printff("%p .. %p = %zu", bufws.str, str.str, bufws.str-str.str-j0);
        j0 += bufws.str - str.str - j0;
        //printf("bufws[%.*s]", STR2_F(bufws));getchar();
        //size_t nws = str2_find_nws(bufws);
        //j0 += nws;
        //jE += nws;
        //if(jE > len) jE = len;
        /* get the buffer to show */
        size_t inof = str2_index_nof(bufws, w);
        size_t jE = inof < str2_len(bufws) ? inof : str2_len(bufws);
        Str2 buf = str2_iE(bufws, jE);
        /* get the format */
        Str2 fmt = {0};
        size_t x = str2_rfind_f0(buf, &fmt);
#if 0
        size_t len_nof = w;
        do {
            //j0 += (w - len_nof);
            buf = str2_i0iE(str, j0, jE);
            jE += (w - len_nof);
            //printf("get %zu,%zu/len %zu\n", j0,jE,len);
            //printf("{%.*s@%zu:%zu}", STR2_F(buf), j0, jE);
            len_nof = str2_len_nof(buf);
            if(jE >= len) break;
        } while(len_nof < w || jE >= len);
        if(jE > len) jE = len;
        //printf("get2 %zu,%zu/len %zu\n", j0,jE,len);
        str2_i0iE(str, j0, jE);
        printf("%zu",len_nof);
#endif
        printf("[");
        str2_print(p->fmt);
        //str2_printraw(buf);
        //printf("|");
        str2_print(buf);
        if(str2_len(fmt) || str2_len(p->fmt)) printf("\033[0m");
        printf("]");
        printf("\n");
        if(x < str2_len(buf)) {
            p->fmt = fmt;
        }
#if 0
        //printf("<%.*s@%zu:%zu>", STR2_F(buf), j0, jE);
        if(str2_len(buf)) {
            printf("[ ");
            str2_printraw(p->fmt);
            printf(" | ");
            str2_printraw(buf);
            //printf("%.*s", STR2_F(buf));
#if 1
            if(inof != w) {
                printf("!!!!!FMT!!!!!");
            }
#else
            if(len_nof != str2_len(buf)) {
                //size_t fE, f0 = str2_rfind_f(buf, &fE);
                p->fmt = str2_rfind_f0(buf);
                j0 += str2_len(buf) - len_nof;
                //if(f0 < str2_len(buf)) {
                //    p->fmt = str2_i0iE(buf, f0, fE);
                //}
            }
#endif
            if(str2_len(p->fmt)) {
                printf("\033[0m");
            }
            printf(" ]");
            p->nl_pending = true;
        }
#endif
        j0 += jE;
    }
}

void str2_printraw(Str2 str) { /*{{{*/
    for(size_t i = 0; i < str2_len(str); ++i) {
        char c = str2_at(str, i);
        if(c <= 0x1f) {
            printf("\\x%2x", (unsigned char)c);
        } else {
            printf("%c", c);
        }
    }
} /*}}}*/

