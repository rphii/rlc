#ifndef STR2_H

#include "array.h"
#include "err.h"
#include "vec.h"
#include "color.h"
#include <stdarg.h>

#define STR2_F(s)           (int)(s).len, (s).str
//#define STR2_BIT_HEAP       (~(SIZE_MAX >> 1))
//#define STR2_BIT_DYNAMIC    (~(SIZE_MAX >> 2) & (SIZE_MAX >> 1))
//#define STR2_BIT_MASK       (~(SIZE_MAX >> 2))

typedef struct Str2 {
    char *str;
    struct {
        size_t len      : -2+8*sizeof(size_t);
        bool is_dynamic : +1;
        bool is_heap    : +1;
    };
    size_t hash_val;
    void *hash_src;
} Str2, Str2C, *VStr2, *VStr2C;


#if 0
#define IMPL_STR_POP_BACK_CHAR(A, N) /**/ \
#define IMPL_STR_POP_BACK_WORD(A, N) /**/ \
#define IMPL_STR_RREMOVE_CH(A, N) /**/ \
#define IMPL_STR_INDEX_NOF(A, N) /**/ find_nof ! \
int str_cmp_ci_any(const Str *a, const Str **b, size_t len) { /**/
ErrDecl str_remove_escapes(Str *restrict out, Str *restrict in) { /**/
#endif

Str2 str2_dyn(Str2C str);

#define str2(s) str2_ll(s, sizeof(s)-1)
Str2C str2_l(char *str);
Str2C str2_ll(char *str, size_t len);
Str2C str2_i0(Str2 str, size_t i0);
Str2C str2_iE(Str2 str, size_t iE);
Str2C str2_i0iE(Str2 str, size_t i0, size_t iE);
Str2C str2_trim(Str2 str);
Str2C str2_triml(Str2 str);
Str2C str2_trimr(Str2 str);
Str2C str2_triml_nof(Str2 str);
Str2C str2_ensure_dir(Str2 str);
Str2C str2_get_ext(Str2 str);
Str2C str2_get_noext(Str2 str);
Str2C str2_get_dir(Str2 str);
Str2C str2_get_nodir(Str2 str);
Str2C str2_get_basename(Str2 str);
void str2_as_cstr(Str2 str, char *out, size_t len);
#define ERR_str2_as_bool(...)  "failed converting string to bool"
int str2_as_bool(Str2 str, bool *out);
int str2_as_int(Str2 str, int *out, int base);
int str2_as_size(Str2 str, size_t *out, int base);
#define ERR_str2_as_ssize(...)  "failed converting string to ssize_t"
int str2_as_ssize(Str2 str, ssize_t *out, int base);
int str2_as_float(Str2 str, float *out);
#define ERR_str2_as_double(...)  "failed converting string to double"
int str2_as_double(Str2 str, double *out);
int str2_as_color(Str2 str, Color *out);

bool str2_is_heap(Str2 str);
bool str2_is_dynamic(Str2 str);
size_t str2_len(Str2 str);
size_t str2_len_nof(Str2 str);
size_t str2_dhash(Str2 str);
size_t str2_hash(Str2 *str);
size_t str2_hash_ci(Str2 *str);
int str2_cmp(Str2 a, Str2 b);
int str2_cmp0(Str2 a, Str2 b);
int str2_cmpE(Str2 a, Str2 b);
int str2_cmp_ci(Str2 a, Str2 b);
int str2_cmp0_ci(Str2 a, Str2 b);
int str2_cmpE_ci(Str2 a, Str2 b);
int str2_hcmp(Str2 *a, Str2 *b);
int str2_hcmp_ci(Str2 *a, Str2 *b);

size_t str2_find_f(Str2 str, size_t *out_iE);
size_t str2_find_ch(Str2 str, char c);
size_t str2_find_nch(Str2 str, char c);
size_t str2_find_ws(Str2 str);
size_t str2_find_nws(Str2 str);
size_t str2_find_any(Str2 str, Str2 any);
size_t str2_find_nany(Str2 str, Str2 any);
size_t str2_find_substr(Str2 str, Str2 substr, bool ignorecase);
size_t str2_rfind_f(Str2 str, size_t *out_iE);
size_t str2_rfind_ch(Str2 str, char c);
size_t str2_rfind_nch(Str2 str, char c);
size_t str2_rfind_ws(Str2 str);
size_t str2_rfind_nws(Str2 str);
size_t str2_rfind_any(Str2 str, Str2 any);
size_t str2_rfind_nany(Str2 str, Str2 any);
size_t str2_rfind_substr(Str2 str, Str2 substr, bool ignorecase);
size_t str2_pair_ch(Str2 str, char c1); /* c0 <== str.str[0] */
size_t str2_splice(Str2 to_splice, Str2 *prev, char sep);
size_t str2_index_nof(Str2 str, size_t index);

char str2_at(Str2 str, size_t i);
char *str2_it(Str2 str, size_t i);
size_t str2_count_overlap(Str2 a, Str2 b, bool ignorecase);
size_t str2_count_ch(Str2 str, char c);
size_t str2_count_nch(Str2 str, char c);
size_t str2_count_any(Str2 str, Str2 any);
size_t str2_count_nany(Str2 str, Str2 any);

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
void str2_resize(Str2 *str, size_t len);
void str2_zero(Str2 *str);

typedef struct Str2Print {
    size_t current;
    bool nl_pending;
    Str2 fmt;
} Str2Print;

size_t str2_writefunc(void *ptr, size_t size, size_t nmemb, Str2 *str);
void str2_print(Str2 str);
void str2_println(Str2 str);
void str2_printal(Str2 str, Str2Print *p, size_t i0, size_t iE);
void str2_printraw(Str2 str);

void vstr2_free_set(VStr2 *vstr);

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

