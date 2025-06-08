#ifndef STR_H

#include "array.h"
#include "err.h"
#include "vec.h"
#include "color.h"
#include <stdarg.h>

#define STR_F(s)           (int)(s).len, (s).str
//#define STR_BIT_HEAP       (~(SIZE_MAX >> 1))
//#define STR_BIT_DYNAMIC    (~(SIZE_MAX >> 2) & (SIZE_MAX >> 1))
//#define STR_BIT_MASK       (~(SIZE_MAX >> 2))


typedef struct Str {
    char *str;
    struct {
        size_t len      : -2+8*sizeof(size_t);
        bool is_dynamic : +1;
        bool is_heap    : +1;
    };
#if defined(STR_HASH_ENABLE_CACHED)
    size_t hash_val;
    void *hash_src;
#endif
} Str, StrC, *VStr, *VStrC;


#if 0
#define IMPL_STR_POP_BACK_CHAR(A, N) /**/ \
#define IMPL_STR_POP_BACK_WORD(A, N) /**/ \
#define IMPL_STR_RREMOVE_CH(A, N) /**/ \
#define IMPL_STR_INDEX_NOF(A, N) /**/ find_nof ! \
int str_cmp_ci_any(const Str *a, const Str **b, size_t len) { /**/
ErrDecl str_remove_escapes(Str *restrict out, Str *restrict in) { /**/
#endif

#define STR_DYN()   (Str){ .is_dynamic = true }

Str str_dyn(StrC str);
void str_pdyn(StrC *str);

#define STR(s) (StrC){ .str = s, .len = sizeof(s)-1 }
#define str(s) STR(s)
const StrC str_l(const char *str);
const StrC str_ll(const char *str, size_t len);
const StrC str_i0(Str str, size_t i0);
const StrC str_iE(Str str, size_t iE);
const StrC str_i0iE(Str str, size_t i0, size_t iE);
const StrC str_trim(Str str);
const StrC str_triml(Str str);
const StrC str_trimr(Str str);
const StrC str_triml_nof(Str str);
const StrC str_ensure_dir(Str str);
const StrC str_get_ext(Str str);
const StrC str_get_noext(Str str);
const StrC str_get_dir(Str str);
const StrC str_get_nodir(Str str);
const StrC str_get_basename(Str str);
void str_as_cstr(Str str, char *out, size_t len);
#define ERR_str_as_bool(...)  "failed converting string to bool"
int str_as_bool(Str str, bool *out);
int str_as_int(Str str, int *out, int base);
int str_as_size(Str str, size_t *out, int base);
#define ERR_str_as_ssize(...)  "failed converting string to ssize_t"
int str_as_ssize(Str str, ssize_t *out, int base);
int str_as_float(Str str, float *out);
#define ERR_str_as_double(...)  "failed converting string to double"
int str_as_double(Str str, double *out);
int str_as_color(Str str, Color *out);

bool str_is_heap(Str str);
bool str_is_dynamic(Str str);
size_t str_len(Str str);
size_t str_len_nof(Str str);
size_t str_dhash(Str str);
size_t str_hash(const Str *str);
size_t str_hash_ci(const Str *str);
int str_cmp(Str a, Str b);
int str_cmp_sortable(const Str a, const Str b);
int str_cmp0(Str a, Str b);
int str_cmpE(Str a, Str b);
int str_cmp_ci(Str a, Str b);
int str_cmp0_ci(Str a, Str b);
int str_cmpE_ci(Str a, Str b);
int str_hcmp(const Str *a, const Str *b);
int str_hcmp_ci(const Str *a, const Str *b);
int str_pcmp_sortable(Str *a, Str *b);

size_t str_find_f(Str str, size_t *out_iE);
size_t str_find_ch(Str str, char c);
size_t str_find_nch(Str str, char c);
size_t str_find_ws(Str str);
size_t str_find_nws(Str str);
size_t str_find_any(Str str, Str any);
size_t str_find_nany(Str str, Str any);
size_t str_find_substr(Str str, Str substr, bool ignorecase);
size_t str_rfind_f(Str str, size_t *out_iE);
size_t str_rfind_ch(Str str, char c);
size_t str_rfind_nch(Str str, char c);
size_t str_rfind_ws(Str str);
size_t str_rfind_nws(Str str);
size_t str_rfind_any(Str str, Str any);
size_t str_rfind_nany(Str str, Str any);
size_t str_rfind_substr(Str str, Str substr, bool ignorecase);
size_t str_pair_ch(Str str, char c1); /* c0 <== str.str[0] */
size_t str_splice(Str to_splice, Str *prev, char sep);
size_t str_index_nof(Str str, size_t index);

char str_at(Str str, size_t i);
char *str_it(Str str, size_t i);
size_t str_count_overlap(Str a, Str b, bool ignorecase);
size_t str_count_ch(Str str, char c);
size_t str_count_nch(Str str, char c);
size_t str_count_any(Str str, Str any);
size_t str_count_nany(Str str, Str any);

void str_clear(Str *str);
void str_free(Str *str);
void str_fmt(Str *str, char *format, ...);
void str_fmt_va(Str *str, const char *format, va_list va);
void str_fmt_fgbg(Str *out, Str text, Color fg, Color bg, bool bold, bool italic, bool underline);
void str_fmt_fgbga(Str *out, Str text, Color fg, Color bg, bool bold, bool italic, bool underline);
void str_fmt_websafe(Str *out, Str text);
void str_input(Str *str);
//Str str_copy(Str str);
void str_copy(Str *copy, Str str);
void str_push(Str *str, char c);
void str_extend(Str *str, Str extend);
void str_resize(Str *str, size_t len);
void str_zero(Str *str);

typedef struct StrPrint {
    size_t progress;
    bool nl_pending;
    Str fmt;
} StrPrint;

size_t str_writefunc(void *ptr, size_t size, size_t nmemb, Str *str);
void str_print(Str str);
void str_println(Str str);
void str_printal(Str str, StrPrint *p, size_t i0, size_t iE);
void str_printraw(Str str);

void vstr_free_set(VStr *vstr);

#if 0
#define str_freeall(...) do { \
        for(size_t i = 0; i < sizeof((Str []){__VA_ARGS__}) / sizeof(*(Str []){__VA_ARGS__}); ++i) { \
            printff("Free string %zu: [%.*s]", i, STR_F(((Str []){__VA_ARGS__})[i])); \
            str_free(((Str []){__VA_ARGS__}) + (sizeof(Str)*i) ); \
        } \
    } while(0)
#endif

#define STR_H
#endif

