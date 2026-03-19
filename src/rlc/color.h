#ifndef RL_COLOR_H

#include <stdint.h>

typedef struct RL_Color {
    union {
        uint32_t rgba;
        struct {
            uint8_t a;
            uint8_t b;
            uint8_t g;
            uint8_t r;
        };
    };
} RL_Color;

#define RL_COLOR_RGBA(R,G,B,A)      (RL_Color){ .r = (R), .g = (G), .b = (B), .a = (A)}

#define RL_COLOR_RGB(R,G,B)         (RL_Color){ .r = (R), .g = (G), .b = (B), .a = 0xFF }
#define RL_COLOR_RGB_NEGATIVE(col)  RL_COLOR_RGB(~col.r, ~col.g, ~col.b)

#define RL_COLOR_NONE               RL_COLOR_RGBA(0, 0, 0, 0)
#define RL_COLOR_WHITE 	            RL_COLOR_RGB(0xFF,0xFF,0xFF)
#define RL_COLOR_SILVER 	        RL_COLOR_RGB(0xC0,0xC0,0xC0)
#define RL_COLOR_GRAY 	            RL_COLOR_RGB(0x80,0x80,0x80)
#define RL_COLOR_BLACK 	            RL_COLOR_RGB(0x00,0x00,0x00)
#define RL_COLOR_RED 	            RL_COLOR_RGB(0xFF,0x00,0x00)
#define RL_COLOR_MAROON 	        RL_COLOR_RGB(0x80,0x00,0x00)
#define RL_COLOR_YELLOW 	        RL_COLOR_RGB(0xFF,0xFF,0x00)
#define RL_COLOR_OLIVE 	            RL_COLOR_RGB(0x80,0x80,0x00)
#define RL_COLOR_LIME 	            RL_COLOR_RGB(0x00,0xFF,0x00)
#define RL_COLOR_GREEN 	            RL_COLOR_RGB(0x00,0x80,0x00)
#define RL_COLOR_AQUA 	            RL_COLOR_RGB(0x00,0xFF,0xFF)
#define RL_COLOR_TEAL 	            RL_COLOR_RGB(0x00,0x80,0x80)
#define RL_COLOR_BLUE 	            RL_COLOR_RGB(0x00,0x00,0xFF)
#define RL_COLOR_NAVY 	            RL_COLOR_RGB(0x00,0x00,0x80)
#define RL_COLOR_FUCHSIA 	        RL_COLOR_RGB(0xFF,0x00,0xFF)
#define RL_COLOR_PURPLE 	        RL_COLOR_RGB(0x80,0x00,0x80)

#define RL_COLOR_GAMMA_DEFAULT     2.2

uint8_t rl_color_as_brightness(RL_Color in, double gamma);

#define RL_COLOR_H
#endif

