#include "color.h"
#include <math.h>

uint8_t rl_color_as_brightness(RL_Color in, double gamma) {
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


