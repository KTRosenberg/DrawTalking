//
//  color.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 6/23/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#include "color.hpp"

namespace color {

#if MTT_COLOR_LIGHT_MODE
vec4 BG_COLOR  = WHITE;
vec4 PEN_COLOR = BLACK;
#else
vec4 BG_COLOR  = BLACK;
vec4 PEN_COLOR = WHITE;
#endif

const vec4 make_with_alpha(vec4 rgba, float32 alpha)
{
    return vec4(rgba.r, rgba.g, rgba.b, alpha);
}

}

vec4 HSVtoRGBinplace(int H, double S, double V, float64 alpha)
{
    static float64 output[3];
    HSVtoRGB(H, S, V, output);
    
    return vec4((float32)output[0], (float32)output[1], (float32)output[2], alpha);
}

void HSVtoRGB(int H, double S, double V, float64 output[3])
{
    double C = S * V;
    double X = C * (1 - abs(fmod(H / 60.0, 2) - 1));
    double m = V - C;
    double Rs, Gs, Bs;

    if(H >= 0 && H < 60) {
        Rs = C;
        Gs = X;
        Bs = 0;
    }
    else if(H >= 60 && H < 120) {
        Rs = X;
        Gs = C;
        Bs = 0;
    }
    else if(H >= 120 && H < 180) {
        Rs = 0;
        Gs = C;
        Bs = X;
    }
    else if(H >= 180 && H < 240) {
        Rs = 0;
        Gs = X;
        Bs = C;
    }
    else if(H >= 240 && H < 300) {
        Rs = X;
        Gs = 0;
        Bs = C;
    }
    else {
        Rs = C;
        Gs = 0;
        Bs = X;
    }
    
    output[0] = (Rs + m);
    output[1] = (Gs + m);
    output[2] = (Bs + m);
}


void print_bgra_buffer_as_rgba(uint8 * data, usize width, usize height)
{
    for (usize r = 0; r < height; r += 1) {
        MTT_print("%s", "{");
        for (usize c = 0; c < width; c += 1) {
            MTT_print("[%u,%u,%u,%u]",
                      data[(r * width) + c + 2],
                      data[(r * width) + c + 1],
                      data[(r * width) + c],
                      data[(r * width) + c + 3]);
        }
        MTT_print("%s", "}\n");
    }
    
}

void print_bgra_buffer_as_bgra(uint8 * data, usize width, usize height)
{
    for (usize r = 0; r < height; r += 1) {
        MTT_print("%s", "{");
        for (usize c = 0; c < width; c += 1) {
            MTT_print("[%u,%u,%u,%u]",
                      data[(r * width) + c],
                      data[(r * width) + c + 1],
                      data[(r * width) + c + 2],
                      data[(r * width) + c + 3]);
        }
        MTT_print("%s", "}\n");
    }
    
}


