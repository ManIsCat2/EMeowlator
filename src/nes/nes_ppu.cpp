#include "nes_ppu.hpp"
#include "nes_cpu.hpp"
#include <cstring>

PPU ppu;

uint32_t nesPalette[64] = {
    0xFF656565, 0xFF002A84, 0xFF1513A2, 0xFF3A019E, 0xFF59007A, 0xFF6A003E, 0xFF680800, 0xFF531D00, 0xFF323400, 0xFF0D4600, 0xFF004F00, 0xFF004C09, 0xFF003F4B, 0xFF000000, 0xFF000000, 0xFF000000,
    0xFFAEAEAE, 0xFF175FD6, 0xFF4341FF, 0xFF7529FA, 0xFF9E1DCA, 0xFFB4207B, 0xFFB13322, 0xFF964E00, 0xFF6A6C00, 0xFF398400, 0xFF0F9000, 0xFF008D33, 0xFF007B8C, 0xFF000000, 0xFF000000, 0xFF000000,
    0xFFFEFFFF, 0xFF66AFFF, 0xFF9390FF, 0xFFC578FF, 0xFFEE6CFF, 0xFFFF6FCA, 0xFFFF8271, 0xFFE69E25, 0xFFBABC00, 0xFF88D501, 0xFF5EE132, 0xFF47DD82, 0xFF4ACBDC, 0xFF4E4E4E, 0xFF000000, 0xFF000000,
    0xFFFEFFFF, 0xFFC0DEFF, 0xFFD2D1FF, 0xFFE7C7FF, 0xFFF8C2FF, 0xFFFFC3E9, 0xFFFFCBC4, 0xFFF5D7A5, 0xFFE2E394, 0xFFCEED96, 0xFFBCF2AA, 0xFFB3F1CB, 0xFFB4E9F0, 0xFFB6B6B6, 0xFF000000, 0xFF000000,
};

uint32_t nesPaletteDefault[64] = {
    0xFF656565, 0xFF002A84, 0xFF1513A2, 0xFF3A019E, 0xFF59007A, 0xFF6A003E, 0xFF680800, 0xFF531D00, 0xFF323400, 0xFF0D4600, 0xFF004F00, 0xFF004C09, 0xFF003F4B, 0xFF000000, 0xFF000000, 0xFF000000,
    0xFFAEAEAE, 0xFF175FD6, 0xFF4341FF, 0xFF7529FA, 0xFF9E1DCA, 0xFFB4207B, 0xFFB13322, 0xFF964E00, 0xFF6A6C00, 0xFF398400, 0xFF0F9000, 0xFF008D33, 0xFF007B8C, 0xFF000000, 0xFF000000, 0xFF000000,
    0xFFFEFFFF, 0xFF66AFFF, 0xFF9390FF, 0xFFC578FF, 0xFFEE6CFF, 0xFFFF6FCA, 0xFFFF8271, 0xFFE69E25, 0xFFBABC00, 0xFF88D501, 0xFF5EE132, 0xFF47DD82, 0xFF4ACBDC, 0xFF4E4E4E, 0xFF000000, 0xFF000000,
    0xFFFEFFFF, 0xFFC0DEFF, 0xFFD2D1FF, 0xFFE7C7FF, 0xFFF8C2FF, 0xFFFFC3E9, 0xFFFFCBC4, 0xFFF5D7A5, 0xFFE2E394, 0xFFCEED96, 0xFFBCF2AA, 0xFFB3F1CB, 0xFFB4E9F0, 0xFFB6B6B6, 0xFF000000, 0xFF000000,
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

    return 0xFF000000 | (r << 16) | (g << 8) | b;
}

void PPU::reset(void) {
    memset(VRAM.data(), 0, VRAM_MIRRORED_SIZE);
    memset(paletteRAM.data(), 0, PALRAM_SIZE);
    memset(frameBuffer, 0, sizeof(frameBuffer));
    WriteLatch = false;
    TransferAddr = 0;
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
                    if (maskRenderSprites) {
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
                    uint32_t fbColor = nesPalette[finalPal];
                    if (finalPal == hoveredPaletteIndex) fbColor = getRainbowColor();
					frameBuffer[ScanLine * NES_WIDTH + Dot] = fbColor;
				}

				if (Dot < 336) {
					shiftRegHigh <<= 1;
					shiftRegLow <<= 1;
					shiftAttribute <<= 2;
				}

				uint16_t fetchAddress = (FullPPUCTRL << 8 & 0x1000) | ntb << 0x04 | VRAMAddr >> 0x0C;
				switch ((Dot) & 7) {
                    case 1:
                        ntb = readVRAM(VRAMAddr);
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
				VRAMAddr = ((VRAMAddr & 0x7be0) | (TransferAddr & 0x41f));
			}
		}

		if (Dot >= 280 && Dot <= 304 && ScanLine == 261) {
            // 0b0000010000011111, 0b0111101111100000
			VRAMAddr = ((VRAMAddr & 0x41f) | (TransferAddr & 0x7be0));
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

bool PPU::Init() {    
    memset(frameBuffer, 0, sizeof(frameBuffer));
    return true;
}