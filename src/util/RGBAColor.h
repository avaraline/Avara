#pragma once

#include <SDL2/SDL.h>

#include <stdint.h>

static void LongToRGBA(long in, float *out, int n = 4)
{
    if (n < 3 || n > 4) {
        SDL_Log("n must be 3 (for RGB) or 4 (for RGBA)");
        exit(69);
    }

    out[0] = ((in >> 16) & 0xFF) / 255.0;
    out[1] = ((in >> 8) & 0xFF) / 255.0;
    out[2] = (in & 0xFF) / 255.0;

    if (n == 4) {
        out[3] = (in >> 24) / 255.0;
    }
}

static void LongToRGBA(long in, int *out, int n = 4)
{
    if (n < 3 || n > 4) {
        SDL_Log("n must be 3 (for RGB) or 4 (for RGBA)");
        exit(69);
    }

    out[0] = (in >> 16) & 0xFF;
    out[1] = (in >> 8) & 0xFF;
    out[2] = in & 0xFF;

    if (n == 4) {
        out[3] = in >> 24;
    }
}
