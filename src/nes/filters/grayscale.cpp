#include "grayscale.hpp"
#include <math.h>

void GrayScaleFilter::applyFilter(uint32_t *pix, UNUSED int x, UNUSED int y) {
    uint32_t pixel = *pix;
    
    uint8_t r = (pixel >> 16) & 0xFF;
    uint8_t g = (pixel >> 8) & 0xFF;
    uint8_t b = pixel & 0xFF;
    
    uint8_t gray = (0.299f * r + 0.587f * g + 0.114f * b);
    
    *pix = (gray << 16) | (gray << 8) | gray;
}
