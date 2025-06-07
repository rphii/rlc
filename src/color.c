#include "color.h"
#include "str.h"

void color_fmt_rgb(Str *out, Color color) { /*{{{*/
    ASSERT_ARG(out);
    if(!str_is_dynamic(*out)) ABORT("attempting to format constant string");
    Str text = STR_DYN();
    str_fmt(&text, "#%02x%02x%02x", color.r, color.g, color.b);
    str_fmt_fgbg(out, text, COLOR_NONE, color, false, false, false);
    str_free(&text);
} /*}}}*/


