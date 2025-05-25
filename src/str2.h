#ifndef STR2_H

#include "array.h"
#include "err.h"
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

#define str2(s) str2_ll(s, sizeof(s)-1)
Str2 str2_l(char *str);
Str2 str2_ll(char *str, size_t len);
Str2 str2_i0(Str2 str, size_t i0);
Str2 str2_iE(Str2 str, size_t iE);
Str2 str2_i0iE(Str2 str, size_t i0, size_t iE);
Str2 str2_trim(Str2 str);
Str2 str2_triml(Str2 str);
Str2 str2_trimr(Str2 str);
Str2 str2_get_ext(Str2 str);
Str2 str2_get_noext(Str2 str);
Str2 str2_get_dir(Str2 str);
Str2 str2_get_nodir(Str2 str);
Str2 str2_get_basename(Str2 str);

bool str2_is_heap(Str2 str);
size_t str2_len(Str2 str);
size_t str2_hash(Str2 *str);
size_t str2_hash_ci(Str2 *str);
int str2_cmp(Str2 a, Str2 b);
int str2_cmp_ci(Str2 a, Str2 b);
int str2_hcmp(Str2 *a, Str2 *b);
int str2_hcmp_ci(Str2 *a, Str2 *b);

size_t str2_find_ch(Str2 x, char c);
size_t str2_rfind_ch(Str2 x, char c);

char str2_at(Str2 str, size_t i);
char *str2_it(Str2 str, size_t i);

void str2_clear(Str2 *str);
void str2_free(Str2 *str);
void str2_fmt(Str2 *str, char *format, ...);
void str2_fmt_va(Str2 *str, const char *format, va_list va);
Str2 str2_copy(Str2 str);
void str2_extend(Str2 *str, Str2 ext);

#define STR2_H
#endif

