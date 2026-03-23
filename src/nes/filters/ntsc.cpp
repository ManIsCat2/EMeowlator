#include "ntsc.hpp"

void NTSCFilter::initialize(void) {
    NTSCSetup._32bit_palette = nesPalette;
    nes_ntsc_init(&NTSC, &NTSCSetup);
}

void NTSCFilter::blit(void) {
    nes_ntsc_blit(&NTSC, ppu.palIndexBuf, 256, 0, 256, 240, ppu.frameBuffer, NES_NTSC_OUT_WIDTH(256) * sizeof(uint32_t));
}