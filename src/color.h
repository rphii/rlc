#ifndef COLOR_H

#include <stdint.h>

typedef struct Color {
    union {
        uint32_t rgba;
        struct {
            uint8_t r;
            uint8_t g;
            uint8_t b;
            uint8_t a;
        };
    };
} Color;

#define COLOR_RGBA(r,g,b,a)     (Color){{{ r, g, b, a }}}

#define COLOR_RGB(r,g,b)        (Color){{{ r, g, b, 0xFF }}}
#define COLOR_RGB_NEGATIVE(col) COLOR_RGB(~col.r, ~col.g, ~col.b)

#define COLOR_NONE              COLOR_RGBA(0, 0, 0, 0)
#define COLOR_WHITE 	        COLOR_RGB(0xFF,0xFF,0xFF)
#define COLOR_SILVER 	        COLOR_RGB(0xC0,0xC0,0xC0)
#define COLOR_GRAY 	            COLOR_RGB(0x80,0x80,0x80)
#define COLOR_BLACK 	        COLOR_RGB(0x00,0x00,0x00)
#define COLOR_RED 	            COLOR_RGB(0xFF,0x00,0x00)
#define COLOR_MAROON 	        COLOR_RGB(0x80,0x00,0x00)
#define COLOR_YELLOW 	        COLOR_RGB(0xFF,0xFF,0x00)
#define COLOR_OLIVE 	        COLOR_RGB(0x80,0x80,0x00)
#define COLOR_LIME 	            COLOR_RGB(0x00,0xFF,0x00)
#define COLOR_GREEN 	        COLOR_RGB(0x00,0x80,0x00)
#define COLOR_AQUA 	            COLOR_RGB(0x00,0xFF,0xFF)
#define COLOR_TEAL 	            COLOR_RGB(0x00,0x80,0x80)
#define COLOR_BLUE 	            COLOR_RGB(0x00,0x00,0xFF)
#define COLOR_NAVY 	            COLOR_RGB(0x00,0x00,0x80)
#define COLOR_FUCHSIA 	        COLOR_RGB(0xFF,0x00,0xFF)
#define COLOR_PURPLE 	        COLOR_RGB(0x80,0x00,0x80)

typedef struct Str Str;
void color_fmt_rgb(Str *out, Color color);
void color_fmt_rgb_fmt(Str *out, Color color, Str fmt);

#define COLOR_H
#endif

