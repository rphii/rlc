#ifndef STR2_H

#include "array.h"
#include "err.h"
#include "color.h"
#include <stdarg.h>

#define STR2(x)     str2
#define STR2_F(s)           (int)s.len, s.str
#define STR2_BIT_DYNAMIC    (~(SIZE_MAX >> 1))

typedef struct Str2 {
    char *str;
    size_t len;
    size_t hash_val;
    void *hash_src;
} Str2;

#if 0
ErrImpl str_fmt_fgbg(Str *out, const Str *text, const V3u8 fg, const V3u8 bg, bool bold, bool italic, bool underline) { /**/
#define IMPL_STR_POP_BACK_CHAR(A, N) /**/ \
#define IMPL_STR_POP_BACK_WORD(A, N) /**/ \
#define IMPL_STR_RREMOVE_CH(A, N) /**/ \
#define STR_IMPL_CSTR(A, N, FMT) /**/ \
#define IMPL_STR_FIND_RFIND_F(A, N, R) /**/ \
#define IMPL_STR_COUNT_OVERLAP(A, N) /**/ \
#define IMPL_STR_COUNT_CH(A, N) /**/ \
#define IMPL_STR_FIND_SUBSTR(A, N) /**/ \
#define IMPL_STR_RFIND_SUBSTR(A, N) /**/ \
#define IMPL_STR_LENGTH_NOF(A, N) /**/ \
#define IMPL_STR_INDEX_NOF(A, N) /**/ find_nof ! \
int str_cmp_ci_any(const Str *a, const Str **b, size_t len) { /**/
size_t str_find_any(const Str *str, const Str *any) { /**/
size_t str_find_nany(const Str *str, const Str *any) { /**/
ErrDecl str_remove_escapes(Str *restrict out, Str *restrict in) { /**/
#endif

#define str2(s) str2_ll(s, sizeof(s)-1)
Str2 str2_l(char *str);
Str2 str2_ll(char *str, size_t len);
Str2 str2_i0(Str2 str, size_t i0);
Str2 str2_iE(Str2 str, size_t iE);
Str2 str2_i0iE(Str2 str, size_t i0, size_t iE);
Str2 str2_trim(Str2 str);
Str2 str2_triml(Str2 str);
Str2 str2_trimr(Str2 str);
Str2 str2_ensure_dir(Str2 str);
Str2 str2_get_ext(Str2 str);
Str2 str2_get_noext(Str2 str);
Str2 str2_get_dir(Str2 str);
Str2 str2_get_nodir(Str2 str);
Str2 str2_get_basename(Str2 str);
void str2_as_cstr(Str2 str, char *out, size_t len);
ErrDecl str2_as_bool(Str2 str, bool *out);
ErrDecl str2_as_int(Str2 str, int *out);
ErrDecl str2_as_size(Str2 str, size_t *out);
ErrDecl str2_as_float(Str2 str, float *out);
ErrDecl str2_as_double(Str2 str, double *out);
ErrDecl str2_as_color(Str2 str, Color *out);

bool str2_is_heap(Str2 str);
size_t str2_len(Str2 str);
size_t str2_hash(Str2 *str);
size_t str2_hash_ci(Str2 *str);
int str2_cmp(Str2 a, Str2 b);
int str2_cmp_ci(Str2 a, Str2 b);
int str2_hcmp(Str2 *a, Str2 *b);
int str2_hcmp_ci(Str2 *a, Str2 *b);

size_t str2_find_ch(Str2 str, char c);
size_t str2_find_nch(Str2 str, char c);
size_t str2_find_ws(Str2 str);
size_t str2_find_nws(Str2 str);
size_t str2_rfind_ch(Str2 str, char c);
size_t str2_rfind_nch(Str2 str, char c);
size_t str2_rfind_ws(Str2 str);
size_t str2_rfind_nws(Str2 str);
size_t str2_pair_ch(Str2 str, char c1); /* c0 <== str.str[0] */
size_t str2_splice(Str2 to_splice, Str2 *prev, char sep);

char str2_at(Str2 str, size_t i);
char *str2_it(Str2 str, size_t i);

void str2_clear(Str2 *str);
void str2_free(Str2 *str);
void str2_fmt(Str2 *str, char *format, ...);
void str2_fmt_va(Str2 *str, const char *format, va_list va);
void str2_fmt_fgbg(Str2 *out, Str2 text, Color fg, Color bg, bool bold, bool italic, bool underline);
void str2_fmt_fgbga(Str2 *out, Str2 text, Color fg, Color bg, bool bold, bool italic, bool underline);
void str2_input(Str2 *str);
Str2 str2_copy(Str2 str);
void str2_push(Str2 *str, char c);
void str2_extend(Str2 *str, Str2 extend);

size_t str2_writefunc(void *ptr, size_t size, size_t nmemb, Str2 *str);

#if 0
#define str2_freeall(...) do { \
        for(size_t i = 0; i < sizeof((Str2 []){__VA_ARGS__}) / sizeof(*(Str2 []){__VA_ARGS__}); ++i) { \
            printff("Free string %zu: [%.*s]", i, STR2_F(((Str2 []){__VA_ARGS__})[i])); \
            str2_free(((Str2 []){__VA_ARGS__}) + (sizeof(Str2)*i) ); \
        } \
    } while(0)
#endif

#define STR2_H
#endif

