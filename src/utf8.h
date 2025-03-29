#ifndef UTF8_H

#include <stdint.h>
#include "err.h"
#include "str.h"

typedef struct U8Point {
    uint32_t val;
    int bytes;
} U8Point;

#define U8_CAP  8

typedef char U8Str[U8_CAP];

void str_u8str(U8Str u8str, Str str);
void rstr_u8str(U8Str u8str, RStr str);
void u8str_rstr(RStr *str, U8Str u8str);

#define ERR_cstr_to_u8_point(...) "failed conversion to u8 point"
ErrDecl cstr_to_u8_point(U8Str in, U8Point *point);

#define ERR_cstr_from_u8_point(...) "failed conversion from u8 point"
ErrDecl cstr_from_u8_point(U8Str out, U8Point *point);

#define UTF8_H
#endif

