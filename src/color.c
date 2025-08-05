#include "color.h"
#include "str.h"
#include <math.h>

void color_fmt_rgb(Str *out, Color color) { /*{{{*/
    ASSERT_ARG(out);
    if(!str_is_dynamic(*out)) ABORT("attempting to format constant string");
    Str text = STR_DYN();
    str_fmt(&text, "#%02x%02x%02x", color.r, color.g, color.b);
    str_fmt_fgbg(out, text, COLOR_NONE, color, false, false, false);
    str_free(&text);
} /*}}}*/

void color_fmt_rgb_fmt(Str *out, Color color, Str fmt) {
    ASSERT_ARG(out);
    if(!str_is_dynamic(*out)) ABORT("attempting to format constant string");
    Str text = STR_DYN();
    str_extend(&text, fmt);
    str_fmt_fgbg(out, text, COLOR_NONE, color, false, false, false);
    str_free(&text);
}

uint8_t color_as_brightness(Color in, double gamma) {
    /*
     * Y = .2126 * R^gamma + .7152 * G^gamma + .0722 * B^gamma
     * L* = 116 * Y ^ 1/3 - 16
     */
    uint8_t result = 0;
    double R = (double)in.r / 255.0;
    double G = (double)in.g / 255.0;
    double B = (double)in.b / 255.0;
    double A = (double)in.a / 255.0;
    double Y = 0.2126 * pow(R, gamma) + 0.7152 * pow(G, gamma) + 0.0722 * pow(B, gamma);
    double L = 116.0 * pow(Y, 1.0/3.0) - 16.0;
    result = (uint8_t)round(A * L);
    return result;
}


