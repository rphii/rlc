#ifndef WSTR_H

#define WSTR_DEFAULT_SIZE 32

#include "str.h"
#include "err.h"

#define VEC_SETTINGS_DEFAULT_SIZE WSTR_DEFAULT_SIZE
#define VEC_SETTINGS_KEEP_ZERO_END 1
#define VEC_SETTINGS_STRUCT_ITEMS s
#include "vec.h"
VEC_INCLUDE(WStr, wstr, wchar_t, BY_VAL, BASE);
VEC_INCLUDE(WStr, wstr, wchar_t, BY_VAL, ERR);
#undef VEC_SETTINGS_DEFAULT_SIZE
#undef VEC_SETTINGS_KEEP_ZERO_END
#undef VEC_SETTINGS_STRUCT_ITEMS

#define WSTR_F(w)   (int)wstr_length((w)), wstr_iter_begin((w))

#define ERR_wstr_from_rstr(...) "error converting rstr to wstr"
ErrDecl wstr_from_rstr(WStr *w, RStr s);

size_t wstr_wlength(WStr w);

#define WSTR_H
#endif

