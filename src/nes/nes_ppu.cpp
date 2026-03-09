#include "nes_ppu.hpp"
#include "nes_cpu.hpp"
#include <cstring>

PPU ppu;

uint32_t nesPalette[64] = {
    0x656565, 0x002A84, 0x1513A2, 0x3A019E, 0x59007A, 0x6A003E, 0x680800, 0x531D00, 0x323400, 0x0D4600, 0x004F00, 0x004C09, 0x003F4B, 0x000000, 0x000000, 0x000000,
    0xAEAEAE, 0x175FD6, 0x4341FF, 0x7529FA, 0x9E1DCA, 0xB4207B, 0xB13322, 0x964E00, 0x6A6C00, 0x398400, 0x0F9000, 0x008D33, 0x007B8C, 0x000000, 0x000000, 0x000000,
    0xFEFFFF, 0x66AFFF, 0x9390FF, 0xC578FF, 0xEE6CFF, 0xFF6FCA, 0xFF8271, 0xE69E25, 0xBABC00, 0x88D501, 0x5EE132, 0x47DD82, 0x4ACBDC, 0x4E4E4E, 0x000000, 0x000000,
    0xFEFFFF, 0xC0DEFF, 0xD2D1FF, 0xE7C7FF, 0xF8C2FF, 0xFFC3E9, 0xFFCBC4, 0xF5D7A5, 0xE2E394, 0xCEED96, 0xBCF2AA, 0xB3F1CB, 0xB4E9F0, 0xB6B6B6, 0x000000, 0x000000,
};

uint32_t nesPaletteDefault[64] = {
    0x656565, 0x002A84, 0x1513A2, 0x3A019E, 0x59007A, 0x6A003E, 0x680800, 0x531D00, 0x323400, 0x0D4600, 0x004F00, 0x004C09, 0x003F4B, 0x000000, 0x000000, 0x000000,
    0xAEAEAE, 0x175FD6, 0x4341FF, 0x7529FA, 0x9E1DCA, 0xB4207B, 0xB13322, 0x964E00, 0x6A6C00, 0x398400, 0x0F9000, 0x008D33, 0x007B8C, 0x000000, 0x000000, 0x000000,
    0xFEFFFF, 0x66AFFF, 0x9390FF, 0xC578FF, 0xEE6CFF, 0xFF6FCA, 0xFF8271, 0xE69E25, 0xBABC00, 0x88D501, 0x5EE132, 0x47DD82, 0x4ACBDC, 0x4E4E4E, 0x000000, 0x000000,
    0xFEFFFF, 0xC0DEFF, 0xD2D1FF, 0xE7C7FF, 0xF8C2FF, 0xFFC3E9, 0xFFCBC4, 0xF5D7A5, 0xE2E394, 0xCEED96, 0xBCF2AA, 0xB3F1CB, 0xB4E9F0, 0xB6B6B6, 0x000000, 0x000000,
};

float rainbowHoverPhase = 0.0f;
uint32_t getRainbowColor() {
    float h = rainbowHoverPhase * 360.0f;
    float s = 1.0f;
    float v = 1.0f;

    float c = v * s;
    float x = c * (1 - fabs(fmod(h / 60.0f, 2) - 1));
    float m = v - c;

    float r1, g1, b1;
    if (h < 60)       { r1 = c; g1 = x; b1 = 0; }
    else if (h < 120) { r1 = x; g1 = c; b1 = 0; }
    else if (h < 180) { r1 = 0; g1 = c; b1 = x; }
    else if (h < 240) { r1 = 0; g1 = x; b1 = c; }
    else if (h < 300) { r1 = x; g1 = 0; b1 = c; }
    else              { r1 = c; g1 = 0; b1 = x; }

    uint8_t r = (uint8_t)((r1 + m) * 255);
    uint8_t g = (uint8_t)((g1 + m) * 255);
    uint8_t b = (uint8_t)((b1 + m) * 255);

    return  (r << 16) | (g << 8) | b;
}

void PPU::reset(void) {
    memset(VRAM.data(), 0, VRAM_MIRRORED_SIZE);
    memset(OAM, 0, 0x100);
    memset(paletteRAM.data(), 0, PALRAM_SIZE);
    memset(frameBuffer, 0, sizeof(frameBuffer));
    WriteLatch = false;
    VRAMAddr = 0;
    OAMAddr = 0;
    TempVRAMAddr = 0;
    ReadBuffer = 0;
    Dot = 0;
    ScanLine = 0;
    Vblank = false;
    sprite0Hit = false;
    
    mask8pxMaskBG = false;
    mask8pxMaskSprites = false;
    maskRenderBG = false;
    maskRenderSprites = false;
    
    nametableSelect = 0; 
    VRAMInc32Mode = false;
    spritePatternTable = 0;
    BGPatternTable = 0;
    use8x16Sprites = false;
    enableNMI = false;
    
    scrollFineX = 0;
}

void PPU::Step() {
	if (Dot == 1 && ScanLine == 241)
		Vblank = true;
	if (Dot == 1 && ScanLine == 261) {
		Vblank = false;
		sprite0Hit = false;
	}

	if (maskRenderBG || maskRenderSprites) {
		if (ScanLine < 240) {
			if (Dot - NES_WIDTH > 63u) {
				if (Dot < NES_WIDTH) {
					uint8_t color = (shiftRegHigh >> (0x0E - scrollFineX) & 0x02) | (shiftRegLow >> (0x0F - scrollFineX) & 0x01);
                    uint8_t palette = shiftAttribute >> (0x1C - scrollFineX * 0x02) & 0x0C;
                    if (maskRenderSprites && !DisableSprites) {
                       for (int i = 0; i < 256; i += 4) {
                            uint8_t *sprite = OAM + i;
                            uint16_t spriteH = use8x16Sprites ? 16 : 8;
                            uint16_t spriteX = Dot - sprite[3];
                            uint16_t spriteY = ScanLine - sprite[0] - 1;

                            int flipH = sprite[2] & 0x40;
                            int flipV = sprite[2] & 0x80;

                            uint16_t sx = spriteX ^ !(flipH) * 7;
                            uint16_t sy = spriteY ^ (flipV ? spriteH - 1 : 0);
                            if (spriteX < 8 && spriteY < spriteH) {
                                uint16_t spriteTile = sprite[1];
                                uint16_t spriteAddress = (use8x16Sprites ? spriteTile % 0x02 << 0x0C | spriteTile << 4 & -32 | sy * 0x02 & 0x10 : (spritePatternTable) << 0x09 | spriteTile << 0x04) | sy & 0x07;
                                uint16_t spriteColor = readCHR(spriteAddress + 8) >> sx << 0x01 & 0x02 | readCHR(spriteAddress) >> sx & 0x01;
                                if (spriteColor) {
                                    if (!(sprite[2] & 0x20 && color)) {
                                        color = spriteColor;
                                        palette = 0x10 | sprite[2] * 0x04 & 0x0C;
                                    }
                                    if (i == 0 && color != 0) sprite0Hit = true;
                                    break;
                                }
                            }
                        }
                    }
                    uint8_t finalPal = paletteRAM[color ? palette | color : 0] & 0x3f;
                    if (finalPal == hoveredPaletteIndex) finalPal = 254;
					palIndexBuf[ScanLine * NES_WIDTH + Dot] = finalPal;
				}

				if (Dot < 336) {
					shiftRegHigh <<= 1;
					shiftRegLow <<= 1;
					shiftAttribute <<= 2;
				}

				uint16_t fetchAddress = ((FullPPUCTRL & 0x10) << 8) | (nametableByte << 4) | ((VRAMAddr >> 12) & 7);
				switch ((Dot) & 7) {
                    case 1:
                        nametableByte = readVRAM(VRAMAddr);
                        break;
                    case 3:
                        attributeByte = (readVRAM((VRAMAddr & 0xc00) | 0x3c0 | (VRAMAddr >> 4 & 0x38) | (VRAMAddr / 4 & 7)) >> ((VRAMAddr >> 5 & 2) | (VRAMAddr / 2 & 1)) * 2) % 4 * 0x5555;
                        break;
                    case 5:
                        patternTableLow = readCHR(fetchAddress);
                        break;
                    case 7: {
                        patternTableHigh = readCHR(fetchAddress + 8);
                        if ((VRAMAddr & 0x001F) == 31) {
                            VRAMAddr &= 0xFFE0;
                            VRAMAddr ^= 0x0400;
                        } else {
                            VRAMAddr++;
                        }
                        shiftRegHigh |= patternTableHigh;
                        shiftRegLow |= patternTableLow;
                        shiftAttribute |= attributeByte;
                        break;
                    }
				}
			}

			if (Dot == 256) {
				if ((VRAMAddr & 0x7000) != 0x7000) {
					VRAMAddr += 0x1000;
				} else {
					VRAMAddr &= 0x0FFF;
					int YScroll = (VRAMAddr & 0x03E0) >> 5;
					if (YScroll == 29) {
						YScroll = 0;
						VRAMAddr ^= 0x0800;
					} else if (YScroll == 31) {// tanks 100th_coin
						YScroll = 0; 
					} else {
						YScroll++;
						YScroll &= 0x1F;
					}
					VRAMAddr = ((VRAMAddr & 0xFC1F) | (YScroll << 5));
				}
			}

			if (Dot == 257) {
                // 0b0111101111100000, 0b0000010000011111
				VRAMAddr = ((VRAMAddr & 0x7be0) | (TempVRAMAddr & 0x41f));
			}
		}

		if (Dot >= 280 && Dot <= 304 && ScanLine == 261) {
            // 0b0000010000011111, 0b0111101111100000
			VRAMAddr = ((VRAMAddr & 0x41f) | (TempVRAMAddr & 0x7be0));
		}

        if (globalROM.mapper) globalROM.mapper->clockPPU();
	}

	Dot++;
	if (Dot > 341) {
		Dot = 0;
		ScanLine++;
		if (ScanLine > 261) ScanLine = 0;
	}
}

uint8_t PPU::readVRAM(uint16_t addr) {
    if (Mirroring == MirrorMode::HORIZONTAL) {
        return VRAM[(addr & 0x3FF) | (addr & 0x800) >> 1];
    } else {
        return VRAM[addr & 0x7FF];
    }
}

uint8_t PPU::readCHR(uint16_t addr) {
    return globalROM.mapper ? globalROM.mapper->ppuRead(addr) : addr;
}

void PPU::LoadCHRROM(const uint8_t *chrData, int chrSize) {
    ChrData.resize(chrSize);
    memcpy(ChrData.data(), chrData, chrSize);
}

void PPU::blitPixels() {
    int size = NES_WIDTH * NES_HEIGHT;
    for (int i = 0; i < size; i++) {
        if (palIndexBuf[i] == 254) frameBuffer[i] = getRainbowColor();
        else frameBuffer[i] = nesPalette[palIndexBuf[i] & 0x3f];
    }
}

void PPU::Init() {
    InitFilter(VideoFilter::NONE);
    memset(frameBuffer, 0, sizeof(frameBuffer));
}

VFilterBase *PPU::GetVideoFilter(VideoFilter filter) {
    switch (filter) {
        case VideoFilter::NONE: return new DefaultFilter();
        case VideoFilter::NTSC: return new NTSCFilter();
        case VideoFilter::CHROMA: return new ChromaFilter();
        case VideoFilter::GRAYSCALE: return new GrayScaleFilter();
    }
    return nullptr;
}

void PPU::InitFilter(VideoFilter filter) {
    filtering = filter;
    if (vfilter) { delete vfilter; vfilter = nullptr; }
    vfilter = GetVideoFilter(filter);
    vfilter->initialize();
}