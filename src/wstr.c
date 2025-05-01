#include <wstr.h>
#include <wchar.h>

#define VEC_SETTINGS_DEFAULT_SIZE WSTR_DEFAULT_SIZE
#define VEC_SETTINGS_KEEP_ZERO_END 1
#define VEC_SETTINGS_STRUCT_ITEMS s
VEC_IMPLEMENT(WStr, wstr, wchar_t, BY_VAL, BASE, 0);
VEC_IMPLEMENT(WStr, wstr, wchar_t, BY_VAL, ERR);
#undef VEC_SETTINGS_DEFAULT_SIZE
#undef VEC_SETTINGS_KEEP_ZERO_END
#undef VEC_SETTINGS_STRUCT_ITEMS

ErrImpl wstr_from_rstr(WStr *w, RStr s) {
    int err = 0;
    Str copy = {0};
    TRYG(rstr_copy(&copy, &s));
    const char *s1 = copy.s;
    const char *s2 = copy.s;
    size_t cap = mbsrtowcs(0, &s1, 0, 0);
    if(cap == (size_t)-1) goto error;
    wstr_clear(w);
    TRYG(wstr_reserve(w, cap));
    // TODO: state
    w->last = cap;// + (cap ? 1 : 0);
    if(cap != mbsrtowcs(w->s, &s2, copy.last, 0)) goto error;
clean:
    str_free(&copy);
    return err;
error:
    ERR_CLEAN;
}

size_t wstr_wlength(WStr w) {
    size_t result = wcswidth(wstr_iter_begin(w), wstr_length(w));
    return result;
}

