#include "grayscale.hpp"
#include <math.h>

void GrayScaleFilter::applyFilter() {
    ppu.blitPixels();
    for (int y = 0; y < NES_HEIGHT; y++) {
        for (int x = 0; x < NES_WIDTH; x++) {
            int i = y * NES_WIDTH + x;
            uint32_t pixel = ppu.frameBuffer[i];

            uint8_t r = (pixel >> 16) & 0xFF;
            uint8_t g = (pixel >> 8)  & 0xFF;
            uint8_t b = pixel & 0xFF;

            uint8_t gray = (0.299f * r + 0.587f * g + 0.114f * b);

            ppu.frameBuffer[i] = (gray << 16) | (gray << 8) | gray;
        }
    }
}
