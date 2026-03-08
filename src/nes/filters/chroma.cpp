#include "chroma.hpp"
#include <math.h>

void ChromaFilter::applyFilter() {
    ppu.blitPixels();
    for (int y = 0; y < NES_HEIGHT; y++) {
        for (int x = 0; x < NES_WIDTH; x++) {
            int i = y * NES_WIDTH + x;
            uint32_t pixel = ppu.frameBuffer[i];

            uint8_t r = (pixel >> 16) & 0xFF;
            uint8_t g = (pixel >> 8)  & 0xFF;
            uint8_t b = pixel & 0xFF;

            float t = (x + y) * 0.02f + 0.61f;;
            float rr = 0.5f + 0.5f * sin(t);
            float gg = 0.5f + 0.5f * sin(t + 2.094f);
            float bb = 0.5f + 0.5f * sin(t + 4.188f);

            r = (uint8_t)(r * rr);
            g = (uint8_t)(g * gg);
            b = (uint8_t)(b * bb);

            ppu.frameBuffer[i] = (r << 16) | (g << 8) | b;
        }
    }
}