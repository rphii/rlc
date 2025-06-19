#define _GNU_SOURCE

#include "str.h"
#include "err.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

void str_resize(Str *str, size_t len) {
    ASSERT_ARG(str);
    if(!str_is_dynamic(*str)) ABORT("attempting to resize constant string");
    char *result = str_is_heap(*str) ? str->str : 0;
    array_resize(result, len + 1);
    if(!str_is_heap(*str)) {
        memcpy(result, str->str, str_len(*str));
    }
    str->str = result;
    str->str[len] = 0;
    str->len = len;
    str->is_dynamic = true;
    str->is_heap = true;
}

void str_zero(Str *str) {
    memset(str, 0, sizeof(*str));
}

#if defined(STR_HASH_ENABLE_CACHED)
#define STR_HASH_PRECOMP(str)  if(str->hash_src == __func__) return str->hash_val
#define STR_HASH_SET(str, hash)        do { \
        ((Str *)str)->hash_src = (void *)__func__; \
        ((Str *)str)->hash_val = hash; \
    } while(0)
#define STR_HASH_CLEAR(str)        do { \
        ((Str *)str)->hash_src = 0; \
    } while(0)
#else
#define STR_HASH_PRECOMP(str)
#define STR_HASH_SET(str, hash)
#define STR_HASH_CLEAR(str)
#endif

void str_pdyn(StrC *str) { /*{{{*/
    str->is_dynamic = true;
} /*}}}*/

Str str_dyn(StrC str) { /*{{{*/
    str.is_dynamic = true;
    //str.len |= STR_BIT_DYNAMIC;
    return str;
} /*}}}*/

const StrC str_l(const char *str) { /*{{{*/
    Str result = { .len = str ? strlen(str) : 0, .str = (char *)str };
    return result;
} /*}}}*/

const StrC str_ll(const char *str, size_t len) { /*{{{*/
    Str result = { .len = len, .str = (char *)str };
    return result;
} /*}}}*/

const StrC str_i0(Str str, size_t i0) { /*{{{*/
    Str result = {0};
    result.str = str_it(str, i0);
    result.len = str_len(str) + str.str - result.str;
    return result;
} /*}}}*/

const StrC str_iE(Str str, size_t iE) { /*{{{*/
    Str result = {0};
    result.str = str.str;
    result.len = str_it(str, iE) - str_it(str, 0);
    return result;
} /*}}}*/

const StrC str_i0iE(Str str, size_t i0, size_t iE) { /*{{{*/
    Str result = {0};
    result.str = str_it(str, i0);
    result.len = str_it(str, iE) - str_it(str, i0);
    return result;
} /*}}}*/

const StrC str_split(Str str, size_t i, Str *right) { /*{{{*/
    size_t len = str_len(str);
    if(i > len) i = len;
    StrC left = str_iE(str, i);
    if(right) *right = str_i0(str, (i + 1 > len) ? len : i + 1);
    return left;
} /*}}}*/

const StrC str_split_ch(Str str, char c, Str *right) { /*{{{*/
    size_t i = str_find_ch(str, c);
    return str_split(str, i, right);
} /*}}}*/

const StrC str_trim(Str str) { /*{{{*/
    Str result = str_triml(str_trimr(str));
    return result;
} /*}}}*/

const StrC str_triml(Str str) { /*{{{*/
    size_t len = str_len(str);
    Str result = str_ll(str.str, len);
    for(size_t i = 0; i < len; ++i) {
        if(!isspace(str_at(result, 0))) break;
        ++result.str;
        --result.len;
    }
    return result;
} /*}}}*/

const StrC str_trimr(Str str) { /*{{{*/
    size_t len = str_len(str);
    Str result = str_ll(str.str, len);
    for(size_t i = len; i > 0; --i) {
        if(!isspace(str_at(result, str_len(result) - 1))) break;
        --result.len;
    }
    return result;
} /*}}}*/

const StrC str_triml_nof(Str str) { /*{{{*/
    size_t len = str_len(str);
    size_t i0 = 0;
    Str result = str_ll(str.str, len);
    size_t m, n = str_find_f(str_i0(result, 0), &m);
    for(size_t i = 0; i < len; ++i) {
        if(i >= m) {
            n = str_find_f(str_i0(result, i), &m);
            n += i;
            m += i;
        }
        if(i >= n && i <= m) continue;
        if(!isspace(str_at(str, i))) break;
        i0 = i + 1;
    }
    return str_i0(result, i0);
} /*}}}*/

const StrC str_ensure_dir(Str str) { /*{{{*/
    // !!! str = str_trim(str);
    size_t len = str_len(str);
    Str result = str_ll(str.str, len);
    if(len > 1) {
        size_t nch = str_rfind_nch(str, PLATFORM_CH_SUBDIR);
        if(!nch && len) ++nch;
        else if(nch < len && str_at(str, nch) != PLATFORM_CH_SUBDIR) ++nch;
        result.len = nch;
        //if(str_at(result, str_len(result) - 1) == PLATFORM_CH_SUBDIR) {
        //    --result.len;
        //}
    }
    return result;
} /*}}}*/

const StrC str_get_ext(Str str) { /*{{{*/
    size_t len = str_len(str);
    Str result = str_ll(str.str, len);
    if(len) {
        size_t i = str_rfind_ch(result, '.');
        if(i < len) {
            /* in case we have something like: file.dir/filename -> / is after . */
            size_t j = str_rfind_ch(result, PLATFORM_CH_SUBDIR);
            if((j < len && j < i) || (j == len)) {
                result.str += i;
                result.len = len - i;
            }
        }
    }
    return result;
} /*}}}*/

const StrC str_get_noext(Str str) { /*{{{*/
    size_t len = str_len(str);
    Str result = str_ll(str.str, len);
    if(len) {
        size_t i = str_rfind_ch(result, '.');
        if(i < len) {
            /* in case we have something like: file.dir/filename -> / is after . */
            size_t j = str_rfind_ch(result, PLATFORM_CH_SUBDIR);
            if((j < len && j < i) || (j == len)) {
                result.len = i;
            }
        }
    }
    return result;
} /*}}}*/

const StrC str_get_dir(Str str) { /*{{{*/
    size_t len = str_len(str);
    Str result = str_ll(str.str, len);
    if(len) {
        size_t i0 = str_rfind_ch(result, '/');
        if(i0 < len && PLATFORM_CH_SUBDIR != '/') {
            i0 = str_rfind_ch(result, PLATFORM_CH_SUBDIR);
        }
        /*if(i0 < len) ++i0;*/
        else if(i0 >= len) i0 = 0;
        result.len = i0;
    }
    return result;
} /*}}}*/

const StrC str_get_nodir(Str str) { /*{{{*/
    size_t len = str_len(str);
    Str result = str_ll(str.str, len);
    if(len) {
        size_t i0 = str_rfind_ch(result, '/');
        if(i0 < len && PLATFORM_CH_SUBDIR != '/') {
            i0 = str_rfind_ch(result, PLATFORM_CH_SUBDIR);
        }
        if(i0 < len) ++i0;
        else if(i0 >= len) i0 = 0;
        result.str += i0;
        result.len -= i0;
    }
    return result;
} /*}}}*/

const StrC str_get_basename(Str str) { /*{{{*/
    size_t len = str_len(str);
    Str result = str_ll(str.str, len);
    if(len) {
        size_t iE = str_rfind_ch(result, '.');
        size_t i0 = str_rfind_ch(result, '/');
        if(i0 < len && PLATFORM_CH_SUBDIR != '/') {
            i0 = str_rfind_ch(result, PLATFORM_CH_SUBDIR);
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

void str_as_cstr(Str str, char *out, size_t len) { /*{{{*/
    ASSERT_ARG(out);
    ASSERT_ARG(len);
    size_t l = str_len(str);
    size_t n = l < len - 1 ? l : len - 1;
    //printff("copying n: %zu [%s] %p -> %p", n, str.str, str.str, out);
    memcpy(out, str.str, n);
    out[n] = 0;
} /*}}}*/

int str_as_bool(Str str, bool *out) { /*{{{*/
    Str in = str_ll(str.str, str_len(str));
    Str val_true[4] = {
        str("true"),
        str("yes"),
        str("y"),
        str("enable"),
    };
    Str val_false[4] = {
        str("false"),
        str("no"),
        str("n"),
        str("disable"),
    };
    bool expand_pool = true;
    bool result = false;
    for(size_t i = 0; i < 4; ++i) {
        if(i && !expand_pool) return -1;
        if(!str_cmp(val_true[i], in)) { result = true; break; }
        if(!str_cmp(val_false[i], in)) { result = false; break; }
        if(i + 1 >= 4 && expand_pool) return -1;
    }
    if(out) *out = result;
    return 0;
} /*}}}*/

int str_as_int(Str str, int *out, int base) { /*{{{*/
    if(!str_len(str)) return -1;
    char *endptr;
    char temp[32] = {0};
    str_as_cstr(str, temp, 32);
    size_t result = strtol(temp, &endptr, base);
    if(endptr - temp != str_len(str)) return -1;
    if(out) *out = result;
    return 0;
} /*}}}*/

int str_as_size(Str str, size_t *out, int base) { /*{{{*/
    if(!str_len(str)) return -1;
    char *endptr;
    char temp[32] = {0};
    str_as_cstr(str, temp, 32);
    size_t result = strtoull(temp, &endptr, base);
    if(endptr - temp != str_len(str)) return -1;
    if(out) *out = result;
    return 0;
} /*}}}*/

int str_as_ssize(Str str, ssize_t *out, int base) { /*{{{*/
    if(!str_len(str)) return -1;
    char *endptr;
    char temp[32] = {0};
    str_as_cstr(str, temp, 32);
    size_t result = strtoll(temp, &endptr, base);
    if(endptr - temp != str_len(str)) return -1;
    if(out) *out = result;
    return 0;
} /*}}}*/

int str_as_float(Str str, float *out) { /*{{{*/
    char *endptr;
    char temp[32] = {0};
    str_as_cstr(str, temp, 32);
    float result = strtof(temp, &endptr);
    if(endptr - temp != str_len(str)) return -1;
    if(out) *out = result;
    return 0;
} /*}}}*/

int str_as_double(Str str, double *out) { /*{{{*/
    char *endptr;
    char temp[32] = {0};
    str_as_cstr(str, temp, 32);
    double result = strtod(temp, &endptr);
    if(endptr - temp != str_len(str)) return -1;
    if(out) *out = result;
    return 0;
} /*}}}*/

int str_as_color(Str str, Color *out) { /*{{{*/
    Color result = {0};
    size_t len = str_len(str);
    size_t hex = 0;
    double dbl = 0;
    bool valid = false;
    if(len >= 7 && str_at(str, 0) == '#') {
        if(str_len(str) == 9) {
            if(str_as_size(str_i0(str, 1), &hex, 16)) return -1;
            result.r = (uint8_t)(hex >> 24);
            result.g = (uint8_t)(hex >> 16);
            result.b = (uint8_t)(hex >> 8);
            result.a = (uint8_t)(hex >> 0);
            valid = true;
        } else if(str_len(str) == 7) {
            if(str_as_size(str_i0(str, 1), &hex, 16)) return -1;
            result.r = (uint8_t)(hex >> 16);
            result.g = (uint8_t)(hex >> 8);
            result.b = (uint8_t)(hex >> 0);
            valid = true;
        }
        else {
            valid = false;
        }
    } else if(len >= 5 && !str_cmp0(str, str("rgb"))) {
        if(str_at(str, 3) == 'a') {
            Str triple = str_i0(str, 4);
            triple = str_i0(triple, str_find_nws(triple));
            if(str_len(triple) && str_at(triple, 0) == '(') {
                size_t iE = str_pair_ch(triple, ')');
                if(iE > str_len(triple) - 1) return -1;
                triple = str_trim(str_i0iE(triple, 1, iE));
                size_t nsep = str_count_ch(triple, ',');
                if(!nsep && str_len(triple) == 8) {
                    if(!str_as_size(triple, &hex, 16)) {
                        result.r = (uint8_t)(hex >> 24);
                        result.g = (uint8_t)(hex >> 16);
                        result.b = (uint8_t)(hex >> 8);
                        result.a = (uint8_t)(hex >> 0);
                        valid = true;
                    }
                } else if(nsep == 3) {
                    size_t i = 0;
                    for(Str splice = {0}; str_splice(triple, &splice, ','); ++i) {
                        if(!splice.str) continue;
                        Str snum = str_trim(splice);
                        if(i < 3) {
                            if(str_as_size(snum, &hex, 10)) return -1;
                            if(i == 0) result.r = hex;
                            if(i == 1) result.g = hex;
                            if(i == 2) result.b = hex;
                        } else {
                            if(str_as_double(snum, &dbl)) return -1;
                            if(i == 3) result.a = (uint8_t)round(0xFF*dbl);
                        }
                    }
                    valid = true;
                }
            }
        } else {
            Str triple = str_i0(str, 3);
            triple = str_i0(triple, str_find_nws(triple));
            if(str_len(triple) && str_at(triple, 0) == '(') {
                size_t iE = str_pair_ch(triple, ')');
                if(iE > str_len(triple) - 1) return -1;
                triple = str_trim(str_i0iE(triple, 1, iE));
                size_t nsep = str_count_ch(triple, ',');
                result.a = 0xFF;
                if(!nsep && str_len(triple) == 6) {
                    if(!str_as_size(triple, &hex, 16)) {
                        result.r = (uint8_t)(hex >> 16);
                        result.g = (uint8_t)(hex >> 8);
                        result.b = (uint8_t)(hex >> 0);
                        valid = true;
                    }
                } else if(nsep == 2) {
                    size_t i = 0;
                    for(Str splice = {0}; str_splice(triple, &splice, ','); ++i) {
                        if(!splice.str) continue;
                        Str snum = str_trim(splice);
                        if(str_as_size(snum, &hex, 10)) return -1;
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

bool str_is_heap(Str str) { /*{{{*/
    return str.is_heap;
} /*}}}*/

bool str_is_dynamic(Str str) { /*{{{*/
    //printf("probing for dyn [%.*s] %p len %zx", STR_F(str), str.str, str.len);
    if(!str.str || str.is_dynamic) {
        //printff(F(" YES", FG_GN));
        return true;
    }
    //printff(F(" NO", FG_RD));
    return false;
} /*}}}*/

size_t str_len(Str str) { /*{{{*/
#if !defined(NDEBUG) && 0
    if(str_is_heap(str)) {
        //printff("str.len %zu, array_len %zu", x.len & ~STR_BIT_HEAP, array_len(x.str));
        ASSERT(str.is_heap ? (str.len + 1 == array_len(str.str)) : 1, "length of dynamic array has to be +1 of actual");
    }
#endif
    return str.len; // & ~STR_BIT_MASK;
} /*}}}*/

size_t str_len_nof(Str str) { /*{{{*/
    size_t len = str_len(str), n = 0, m = 0;
    Str snip = str_ll(str.str, len);
    Str pat = str("\033[");
    for(;;) {
        snip = str_i0(snip, m);
        n = str_find_substr(snip, pat, false);
        if(n >= str_len(snip)) break;
        snip = str_i0(snip, n + str_len(pat));
        m = str_find_ch(snip, 'm');
        len -= (m + str_len(pat));
        if(m++ >= str_len(snip)) break;
        len -= (bool)(m);
    }
    return len;
} /*}}}*/

size_t str_dhash(Str str) { /*{{{*/
    //STR_HASH_PRECOMP(str);
    size_t hash = 5381;
    size_t i = 0;
    while(i < str_len(str)) {
        unsigned char c = str.str[i++];
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    //printff("calculated new hash: %zx [%s]", hash, str->str);
    //STR_HASH_SET(str, hash);
    return hash;
} /*}}}*/

size_t str_hash(const Str *str) { /*{{{*/
    ASSERT_ARG(str);
    STR_HASH_PRECOMP(str);
    size_t hash = 5381;
    size_t i = 0;
    while(i < str_len(*str)) {
        unsigned char c = str->str[i++];
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    //printff("calculated new hash: %zx [%s]", hash, str->str);
    STR_HASH_SET(str, hash);
    return hash;
} /*}}}*/

size_t str_hash_ci(const Str *str) { /*{{{*/
    ASSERT_ARG(str);
    STR_HASH_PRECOMP(str);
    size_t hash = 5381;
    size_t i = 0;
    while(i < str_len(*str)) {
        unsigned char c = tolower(str->str[i++]);
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    //printff("calculated new hash: %zx [%s]", hash, str->str);
    STR_HASH_SET(str, hash);
    return hash;
} /*}}}*/

int str_cmp(Str a, Str b) { /*{{{*/
    size_t la = str_len(a); 
    size_t lb = str_len(b);
    if(la != lb) return -1;
    int result = memcmp(a.str, b.str, la);
    return result;
} /*}}}*/

int str_cmp_sortable(const Str a, const Str b) {
    size_t la = str_len(a);
    size_t lb = str_len(b);
    int result = -1;
    if(la != lb) {
        size_t less = la < lb ? la : lb;
        result = memcmp(a.str, b.str, less);
        if(!result) {
            result = la - lb;
        }
    } else {
        result = memcmp(a.str, b.str, la);
    }
    return result;
} 

int str_cmp0(Str a, Str b) { /*{{{*/
    size_t la = str_len(a); 
    size_t lb = str_len(b);
    if(la < lb) return -1;
    int result = memcmp(a.str, b.str, lb);
    return result;
} /*}}}*/

int str_cmpE(Str a, Str b) { /*{{{*/
    size_t la = str_len(a); 
    size_t lb = str_len(b);
    if(la < lb) return -1;
    int result = memcmp(a.str + la - lb, b.str, lb);
    return result;
} /*}}}*/

int str_cmp_ci(Str a, Str b) { /*{{{*/
    size_t la = str_len(a); 
    size_t lb = str_len(b);
    if(la != lb) return -1;
    for (size_t i = 0; i < la; ++i) {
        int d = tolower(str_at(a, i)) - tolower(str_at(b, i));
        if (d != 0) return d;
    }
    return 0;
} /*}}}*/

int str_cmp0_ci(Str a, Str b) { /*{{{*/
    size_t la = str_len(a); 
    size_t lb = str_len(b);
    if(la < lb) return -1;
    for (size_t i = 0; i < lb; ++i) {
        int d = tolower(str_at(a, i)) - tolower(str_at(b, i));
        if (d != 0) return d;
    }
    return 0;
} /*}}}*/

int str_cmpE_ci(Str a, Str b) { /*{{{*/
    size_t la = str_len(a); 
    size_t lb = str_len(b);
    if(la < lb) return -1;
    for (size_t i = 0; i < lb; ++i) {
        int d = tolower(str_at(a, i + la - lb)) - tolower(str_at(b, i));
        if (d != 0) return d;
    }
    return 0;
} /*}}}*/

int str_hcmp(const Str *a, const Str *b) { /*{{{*/
    ASSERT_ARG(a);
    ASSERT_ARG(b);
    size_t la = str_len(*a); 
    size_t lb = str_len(*b);
    if(la != lb) return -1;
    size_t ha = str_hash(a);
    size_t hb = str_hash(b);
    if(ha != hb) return -1;
    int result = memcmp(a->str, b->str, la);
    return result;
} /*}}}*/

int str_hcmp_ci(const Str *a, const Str *b) { /*{{{*/
    ASSERT_ARG(a);
    ASSERT_ARG(b);
    size_t la = str_len(*a); 
    size_t lb = str_len(*b);
    if(la != lb) return -1;
    size_t ha = str_hash_ci(a);
    size_t hb = str_hash_ci(b);
    if(ha != hb) return -1;
    for (size_t i = 0; i < la; ++i) {
        int d = tolower(str_at(*a, i)) - tolower(str_at(*b, i));
        if (d != 0) return d;
    }
    return 0;
} /*}}}*/

int str_pcmp_sortable(Str *a, Str *b) {
    size_t la = str_len(*a);
    size_t lb = str_len(*b);
    int result = -1;
    if(la != lb) {
        size_t less = la < lb ? la : lb;
        result = memcmp(a->str, b->str, less);
        if(!result) {
            result = la - lb;
        }
    } else {
        result = memcmp(a->str, b->str, la);
    }
    return result;
}

size_t str_find_f(Str str, size_t *out_iE) { /*{{{*/
    size_t i0 = str_find_substr(str, str(FS_BEG), false);
    if(out_iE) {
        Str s = str_i0(str, i0);
        //printf(" find m: [");
        //str_printraw(s);
        //printff("]");
        size_t iE = str_find_ch(s, 'm');
        if(iE < str_len(s)) ++iE;
        *out_iE = i0 + iE;
    }
    return i0;
} /*}}}*/

size_t str_find_ch(Str str, char c) { /*{{{*/
#if 1
    size_t len = str_len(str);
    char *s = memchr(str.str, c, len);
    if(!s) return len;
    return s - str.str;
#else
    for(size_t i = 0; i < str_len(str); ++i) {
        if(str.str[i] == c) return i;
    }
    return str_len(str);
#endif
} /*}}}*/

size_t str_find_nch(Str str, char c) { /*{{{*/
    for(size_t i = 0; i < str_len(str); ++i) {
        if(str.str[i] != c) return i;
    }
    return str_len(str);
} /*}}}*/

size_t str_find_ws(Str str) { /*{{{*/
    for(size_t i = 0; i < str_len(str); ++i) {
        if(isspace(str.str[i])) return i;
    }
    return str_len(str);
} /*}}}*/

size_t str_find_nws(Str str) { /*{{{*/
    for(size_t i = 0; i < str_len(str); ++i) {
        if(!isspace(str.str[i])) return i;
    }
    return str_len(str);
} /*}}}*/

size_t str_find_any(Str str, Str any) { /*{{{*/
    size_t len = str_len(str);
    size_t len2 = str_len(any);
    for(size_t i = 0; i < len; ++i) {
        char c = str.str[i];
        if(memchr(any.str, c, len2)) {
            return i;
        }
    }
    return len;
} /*}}}*/

size_t str_find_nany(Str str, Str any) { /*{{{*/
    size_t len = str_len(str);
    size_t len2 = str_len(any);
    for(size_t i = 0; i < len; ++i) {
        char c = str.str[i];
        if(!memchr(any.str, c, len2)) {
            return i;
        }
    }
    return len;
} /*}}}*/

size_t str_find_substr(Str str, Str sub, bool ignorecase) { /*{{{*/
    /* basic checks */
    if(!str_len(sub)) return 0;
    if(str_len(sub) > str_len(str)) {
        return str_len(str);
    }
    /* store original indices */
    Str ref = str;
    /* check for substring */
    size_t i = 0;
    while(str_len(sub) <= str_len(ref)) {
        size_t overlap = str_count_overlap(ref, sub, ignorecase);
        if(overlap == str_len(sub)) {
            return i;
        } else {
            i += overlap + 1;
            ref.str += overlap + 1;
            ref.len -= overlap + 1;
        }
    }
    /* restore original */
    return str_len(str);
} /*}}}*/

size_t str_rfind_f(Str str, size_t *out_iE) { /*{{{*/
    size_t i0 = str_rfind_substr(str, str(FS_BEG), false);
    if(out_iE) {
        Str s = str_i0(str, i0);
        size_t iE = str_rfind_ch(s, 'm');
        if(iE < str_len(s)) ++iE;
        *out_iE = i0 + iE;
    }
    return i0;
} /*}}}*/

size_t str_rfind_f0(Str str, StrC *fmt) { /*{{{*/
    size_t i0 = str_rfind_substr(str, str(FS_BEG), false);
    Str s = str_i0(str, i0);
    size_t iE = str_find_ch(s, 'm');
    if(iE < str_len(s)) ++iE;
    s = str_iE(s, iE);
    //printf("<FMT:%.*s>",STR_F(s));
    if(!str_cmpE(s, str("[0m"))) str_clear(&s);
    if(fmt) *fmt = s;
    return i0;
} /*}}}*/

size_t str_rfind_ch(Str str, char c) { /*{{{*/
#if 1
    size_t len = str_len(str);
    char *s = memrchr(str.str, c, len);
    if(!s) return len;
    return s - str.str;
#else
    for(size_t i = str_len(str); i > 0; --i) {
        if(str.str[i - 1] == c) return i - 1;
    }
    return str_len(str);
#endif
} /*}}}*/

size_t str_rfind_nch(Str str, char c) { /*{{{*/
    for(size_t i = str_len(str); i > 0; --i) {
        if(str.str[i - 1] != c) return i - 1;
    }
    return 0;
} /*}}}*/

size_t str_rfind_ws(Str str) { /*{{{*/
    for(size_t i = str_len(str); i > 0; --i) {
        if(isspace(str.str[i - 1])) return i - 1;
    }
    return str_len(str);
} /*}}}*/

size_t str_rfind_nws(Str str) { /*{{{*/
    for(size_t i = str_len(str); i > 0; --i) {
        if(!isspace(str.str[i - 1])) return i - 1;
    }
    return str_len(str);
} /*}}}*/

size_t str_rfind_any(Str str, Str any) { /*{{{*/
    size_t len = str_len(str);
    size_t len2 = str_len(any);
    for(size_t i = len; i > 0; --i) {
        char c = str.str[i - 1];
        if(memchr(any.str, c, len2)) {
            return i - 1;
        }
    }
    return len;
} /*}}}*/

size_t str_rfind_nany(Str str, Str any) { /*{{{*/
    size_t len = str_len(str);
    size_t len2 = str_len(any);
    for(size_t i = len; i > 0; --i) {
        char c = str.str[i - 1];
        if(!memchr(any.str, c, len2)) {
            return i - 1;
        }
    }
    return len;
} /*}}}*/

size_t str_rfind_substr(Str str, Str substr, bool ignorecase) { /*{{{*/
    /* basic checks */
    size_t n = str_len(substr);
    size_t m = str_len(str);
    if(!n) return 0;
    if(n > m) {
        return m;
    }
    const char *s = substr.str;
    for(size_t i = m - n + 1; i > 0; --i) {
        const char *t = str_it(str, i - 1);
        if(!memcmp(s, t, n)) return i - 1;
    }
    return m;
} /*}}}*/

size_t str_pair_ch(Str str, char c1) { /*{{{*/
    size_t len = str_len(str);
    if(!len) return len;
    size_t level = 1;
    char c0 = str_at(str, 0);
    for(size_t i = 1; i < str_len(str); ++i) {
        char c = str_at(str, i);
        if(c == c1) level--;
        else if(c == c0) level++;
        if(level <= 0) return i;
    }
    return len;
} /*}}}*/

size_t str_splice(Str to_splice, Str *prev, char sep) { /*{{{*/
    ASSERT_ARG(prev);
    size_t len = str_len(to_splice);
    Str result = str_ll(to_splice.str, len);
    if(prev && prev->str) {
        size_t from = prev->str - to_splice.str + str_len(*prev);
        Str search = str_i0(to_splice, from);
        size_t offset = str_find_ch(search, sep) + from;
        result.str += offset;
        result.len -= offset;
        if(result.str - to_splice.str < str_len(to_splice)) {
            ++result.str;
            --result.len;
        }
    }
    result.len = str_find_ch(result, sep);
    *prev = result;
    return result.str - to_splice.str - len;
} /*}}}*/

size_t str_index_nof(Str str, size_t index) {
    size_t len = str_len(str);
    size_t len_nof = 0;
    size_t n = 0, m = 0, i = 0;
    Str snip = str;
    if(!index) return 0;
    for(;;) {
        snip = str_i0(str, i);
        n = str_find_f(snip, &m);
        if(len_nof + n >= index) {
            //printff("DONE %zu + %zu - %zu", i, index, len_nof);
            return i + (index - len_nof);
        }
        len_nof += n;
        i += m;
        if(i >= len) break;
    }
    return str_len(str);
}

char str_at(Str str, size_t i) { /*{{{*/
    ASSERT(i < str_len(str), "out of bounds: %zu @ [0..%zu)", i, str_len(str));
    return str.str[i];
} /*}}}*/

char *str_it(Str str, size_t i) { /*{{{*/
    ASSERT(i <= str_len(str), "out of bounds: %zu @ [0..%zu]", i, str_len(str));
    return &str.str[i];
} /*}}}*/

size_t str_count_overlap(Str a, Str b, bool ignorecase) { /*{{{*/
    size_t overlap = 0;
    size_t len = str_len(a) > str_len(b) ? str_len(b) : str_len(a);
    if(!ignorecase) {
        for(size_t i = 0; i < len; ++i) {
            char ca = str_at(a, i);
            char cb = str_at(b, i);
            if(ca == cb) ++overlap;
            else break;
        }
    } else {
        for(size_t i = 0; i < len; ++i) {
            int ca = tolower(str_at(a, i));
            int cb = tolower(str_at(b, i));
            if(ca == cb) ++overlap;
            else break;
        }
    }
    return overlap;
} /*}}}*/

size_t str_count_ch(Str str, char c) { /*{{{*/
    size_t result = 0;
    size_t len = str_len(str);
    for(size_t i = 0; i < len; ++i) {
        if(str.str[i] == c) ++result;
    }
    return result;
} /*}}}*/

size_t str_count_nch(Str str, char c) { /*{{{*/
    size_t result = 0;
    size_t len = str_len(str);
    for(size_t i = 0; i < len; ++i) {
        if(str.str[i] != c) ++result;
    }
    return result;
} /*}}}*/

size_t str_count_any(Str str, Str any) { /*{{{*/
    size_t result = 0;
    size_t len = str_len(str);
    size_t len2 = str_len(any);
    for(size_t i = 0; i < len; ++i) {
        if(memchr(any.str, str.str[i], len2)) ++result;
    }
    return result;
} /*}}}*/

size_t str_count_nany(Str str, Str any) { /*{{{*/
    size_t result = 0;
    size_t len = str_len(str);
    size_t len2 = str_len(any);
    for(size_t i = 0; i < len; ++i) {
        if(!memchr(any.str, str.str[i], len2)) ++result;
    }
    return result;
} /*}}}*/

void str_clear(Str *str) { /*{{{*/
    ASSERT_ARG(str);
    str->len = 0;
    STR_HASH_CLEAR(str);
} /*}}}*/

void str_free(Str *str) { /*{{{*/
    if(!str) return;
    //printf("free %p [%.*s]\n", str, STR_F(*str));
    if(str_is_heap(*str)) {
        array_free(str->str);
    }
    memset(str, 0, sizeof(*str));
} /*}}}*/

void str_fmt(Str *str, char *format, ...) { /*{{{*/
    ASSERT_ARG(str);
    ASSERT_ARG(format);
    va_list va;
    va_start(va, format);
    str_fmt_va(str, format, va);
    va_end(va);
} /*}}}*/

void str_fmt_va(Str *str, const char *format, va_list va) { /*{{{*/
    ASSERT_ARG(str);
    ASSERT_ARG(format);
    if(!str_is_dynamic(*str)) ABORT("attempting to format constant string");
    va_list argp2;
    va_copy(argp2, va);
    size_t len_app = (size_t)vsnprintf(0, 0, format, argp2);
    va_end(argp2);

    if((int)len_app < 0) {
        ABORT("len_app is < 0!");
    }

    // calculate required memory
    size_t len_old = str_len(*str);
    size_t len_new = len_old + len_app;
    str_resize(str, len_new);

    // actual append
    int len_chng = vsnprintf(&str->str[len_old], len_app + 1, format, va);

    // check for success
    if(!(len_chng >= 0 && (size_t)len_chng <= len_app)) {
        ABORT("len_chng is < 0!");
    }

    STR_HASH_CLEAR(str);
} /*}}}*/

void str_fmt_fgbg(Str *out, const StrC text, Color fg, Color bg, bool bold, bool italic, bool underline) { /*{{{*/
    ASSERT_ARG(out);
    if(!str_is_dynamic(*out)) ABORT("attempting to format constant string");
    bool do_fmt = ((fg.rgba || bg.rgba || bold || italic || underline));
    if(!do_fmt) {
        str_fmt(out, "%.*s", STR_F(text));
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
    if(fg.rgba && bg.rgba) { str_fmt(out, fmt, fg.r, fg.g, fg.b, bg.r, bg.g, bg.b, STR_F(text)); }
    else if(fg.rgba) {       str_fmt(out, fmt, fg.r, fg.g, fg.b, STR_F(text)); }
    else if(bg.rgba) {       str_fmt(out, fmt, bg.r, bg.g, bg.b, STR_F(text)); }
    else {                   str_fmt(out, fmt, STR_F(text)); }
} /*}}}*/

void str_fmt_fgbga(Str *out, const StrC text, Color fg, Color bg, bool bold, bool italic, bool underline) { /*{{{*/
    ASSERT_ARG(out);
    if(!str_is_dynamic(*out)) ABORT("attempting to format constant string");
    bool do_fmt = ((fg.rgba || bg.rgba || bold || italic || underline));
    if(!do_fmt) {
        str_fmt(out, "%.*s", STR_F(text));
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
    if(fg.rgba && bg.rgba) { str_fmt(out, fmt, (uint8_t)roundf(((float)fg.r*fg.a)/255.0f), (uint8_t)roundf(((float)fg.g*fg.a)/255.0f), (uint8_t)roundf(((float)fg.b*fg.a)/255.0f), (uint8_t)roundf(((float)bg.r*bg.a)/255.0f), (uint8_t)roundf(((float)bg.g*bg.a)/255.0f), (uint8_t)roundf(((float)bg.b*bg.a)/255.0f), STR_F(text)); }
    else if(fg.rgba) {       str_fmt(out, fmt, (uint8_t)roundf(((float)fg.r*fg.a)/255.0f), (uint8_t)roundf(((float)fg.g*fg.a)/255.0f), (uint8_t)roundf(((float)fg.b*fg.a)/255.0f), STR_F(text)); }
    else if(bg.rgba) {       str_fmt(out, fmt, (uint8_t)roundf(((float)bg.r*bg.a)/255.0f), (uint8_t)roundf(((float)bg.g*bg.a)/255.0f), (uint8_t)roundf(((float)bg.b*bg.a)/255.0f), STR_F(text)); }
    else {                   str_fmt(out, fmt, STR_F(text)); }
} /*}}}*/

void str_fmt_websafe(Str *out, Str text) { /*{{{*/
    Str escape = str(" <>#%+{}|\\^~[]';/?:@=&$");
    for(size_t i = 0; i < str_len(text); ++i) {
        unsigned char c = str_at(text, i);
        if(str_find_ch(escape, c) < str_len(escape)) {
            str_fmt(out, "%%%02x", c);
        } else {
            str_push(out, c);
        }
    }
} /*}}}*/

void str_fmtx(Str *out, StrFmtX fmtx, char *fmt, ...) {
    ASSERT_ARG(out);
    ASSERT_ARG(fmt);
    size_t len_old = out->len;
    va_list va;
    va_start(va, fmt);
    str_fmt_va(out, fmt, va);
    va_end(va);
    size_t len_new = out->len;
    str_fmt_fgbg(out, str_i0(*out, len_old), fmtx.fg, fmtx.bg, fmtx.bold, fmtx.italic, fmtx.underline);
    size_t len_diff = out->len - len_new;
    //printff("len old %zu, new %zu, diff %zu", len_old, len_new, len_diff);
    //printff("PART STRING:[%.*s]:%zu", STR_F(*out), out->len);
    //printff("MOVE: %p -> %p x %zu", str_it(*out, len_new), str_it(*out, len_old), len_diff);
    //memmove(str_it(*out, len_old), str_it(*out, len_new), len_diff);
    str_resize(out, out->len - (len_new - len_old));
    //printff("FINAL STRING:[%.*s]:%zu", STR_F(*out), out->len);
}

void str_input(Str *str) { /*{{{*/
    ASSERT_ARG(str);
    if(!str_is_dynamic(*str)) ABORT("attempting to input constant string");
    int c = 0;
    while((c = getchar()) != '\n' && c != EOF) {
        str_push(str, c);
    }
    if(!str_len(*str) && (!c || c == EOF || c == '\n')) {
        //THROW("an error"); /* TODO describe this error. is this even an error? */
    }
    fflush(stdin);
} /*}}}*/

void str_copy(Str *copy, Str str) { /*{{{*/
    str_clear(copy);
    str_extend(copy, str);
} /*}}}*/

void str_push(Str *str, char c) { /*{{{*/
    ASSERT_ARG(str);
    if(!str_is_dynamic(*str)) ABORT("attempting to push constant string");
    size_t len = str_len(*str);
    str_resize(str, len + 1);
    str->str[len] = c;
} /*}}}*/

void str_extend(Str *str, Str extend) { /*{{{*/
    ASSERT_ARG(str);
    //printff("EXTEND");
    //printff("EXTEND");
    if(!str_is_dynamic(*str)) ABORT("attempting to extend constant string");
    //printff("EXTEND");
    size_t len_app = str_len(extend);

    //printff("EXTEND");
    // calculate required memory
    size_t len_old = str_len(*str);
    //printff("EXTEND");
    size_t len_new = len_old + len_app;
    //printff("EXTEND, resize %zu", len_new);
    str_resize(str, len_new);

    //printff("%p .. %p < %p .. %p", str->str,str->str+len_app, extend.str,extend.str+len_app);
    //printff("EXTEND, len_old %zu, %p", len_old, str->str);
    // actual append
    memcpy(&str->str[len_old], extend.str, len_app);

    //printff("EXTEND");
    STR_HASH_CLEAR(str);
} /*}}}*/

size_t str_writefunc(void *ptr, size_t size, size_t nmemb, Str *str) { /*{{{*/
    str_fmt(str, "%.*s", size * nmemb, ptr);
    return size * nmemb;
} /*}}}*/

void vstr_free_set(VStr *vstr) { /*{{{*/
    ASSERT_ARG(vstr);
    array_free_set(*vstr, Str, (ArrayFree)str_free);
} /*}}}*/

void str_print(Str str) { /*{{{*/
    printf("%.*s", STR_F(str));
} /*}}}*/

void str_println(Str str) { /*{{{*/
    printf("%.*s\n", STR_F(str));
} /*}}}*/

void str_printal(Str str, StrPrint *p, size_t i0, size_t iE) {
    ASSERT_ARG(p);
    if(iE <= i0) return;
    bool first = true;
    size_t len = str_len(str);
    size_t w = iE - i0;
    size_t w0 = iE > p->progress ? iE - p->progress : w;
    //printff("w0 %zu",w0);
    size_t pad = i0 > p->progress ? i0 - p->progress : 0;
    w0 -= pad;
    //printff(".");getchar();
    //printff("progress:%zu, pad:%zu, w0:%zu",p->progress,pad,w0);
    printf("%*s", (int)pad, "");
    p->progress += pad;
    //printff(".");getchar();
    for(size_t j0 = 0; j0 < len; ) {
        if(p->nl_pending) {
            p->nl_pending = false;
            p->progress = 0;
            //printff("NL");
            printf("\n");
        }
        //if(jE > len) jE = len;
        Str bufws = first ? str : str_triml_nof(str_i0(str, j0));
        if(!str_len(bufws)) break;
        //printff("%p .. %p = %zu", bufws.str, str.str, bufws.str-str.str-j0);
        j0 += bufws.str - str.str - j0;
        size_t inof = str_index_nof(bufws, first ? w0 : w);
        size_t jE = inof < str_len(bufws) ? inof : str_len(bufws);
        Str buf = str_iE(bufws, jE);
        size_t lnof = str_len_nof(buf);
        /* get the format */
        Str fmt = {0};
        size_t x = str_rfind_f0(buf, &fmt);
        //printf("[");
        if(!first) printf("%*s", (int)(i0), "");
        if(!first) p->progress += i0;
        str_print(p->fmt);
        str_print(buf);
        if(str_len(fmt) || str_len(p->fmt)) printf("\033[0m");
        //printf("]");
        p->progress += lnof;
        if(p->progress >= iE) {
            p->nl_pending = true;
        }
        if(x < str_len(buf)) {
            p->fmt = fmt;
        }
        j0 += jE;
        first = false;
    }
    p->nl_pending = false;
}

void str_printraw(Str str) { /*{{{*/
    //printf("raw[");
    for(size_t i = 0; i < str_len(str); ++i) {
        char c = str_at(str, i);
        if(c <= 0x1f) {
            printf("\\x%2x", (unsigned char)c);
        } else {
            printf("%c", c);
        }
    }
    //printf("]\n");
} /*}}}*/

