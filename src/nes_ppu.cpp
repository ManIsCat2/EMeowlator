#include "nes_ppu.hpp"
#include "nes_cpu.hpp"
#include <cstring>

PPU ppu;

//#define PPU_FUCKEDUP_TIMING

void PPU::Step() {
#ifndef PPU_FUCKEDUP_TIMING
    if (Dot == 1 && ScanLine == 241) Vblank = true;
    if (Dot == 1 && ScanLine == 261) Vblank = false;
#endif
    Dot++;
#ifdef PPU_FUCKEDUP_TIMING
    if (Dot > 340) {
            Dot = 0;
            ScanLine++;
            if (ScanLine > 241) {
                Vblank = true;
            }
            if (ScanLine > 261) {
                ScanLine = 0;
                Vblank = false;
            }
        }
#else
    if (Dot > 341) {
        Dot = 0;
        ScanLine++;
        if (ScanLine > 261) {
            ScanLine = 0;
        }
    }
#endif
}

void PPU::LoadCHRROM(const uint8_t* chrData, int chrSize) {
    ChrData.resize(chrSize);
    memcpy(ChrData.data(), chrData, chrSize);
}


SDL_Texture* texture = nullptr;

bool PPU::InitSDL(SDL_Renderer * renderer) {
    PaletteMode = 0;
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                SDL_TEXTUREACCESS_STREAMING, NES_WIDTH, NES_HEIGHT);
    return texture != nullptr;
}

void PPU::ShutdownSDL() {
    if (texture) SDL_DestroyTexture(texture);
}

uint32_t nesPaletteDefault[64] = {
    0xFF656565, 0xFF002A84, 0xFF1513A2, 0xFF3A019E, 0xFF59007A, 0xFF6A003E, 0xFF680800, 0xFF531D00, 0xFF323400, 0xFF0D4600, 0xFF004F00, 0xFF004C09, 0xFF003F4B, 0xFF000000, 0xFF000000, 0xFF000000,
    0xFFAEAEAE, 0xFF175FD6, 0xFF4341FF, 0xFF7529FA, 0xFF9E1DCA, 0xFFB4207B, 0xFFB13322, 0xFF964E00, 0xFF6A6C00, 0xFF398400, 0xFF0F9000, 0xFF008D33, 0xFF007B8C, 0xFF000000, 0xFF000000, 0xFF000000,
    0xFFFEFFFF, 0xFF66AFFF, 0xFF9390FF, 0xFFC578FF, 0xFFEE6CFF, 0xFFFF6FCA, 0xFFFF8271, 0xFFE69E25, 0xFFBABC00, 0xFF88D501, 0xFF5EE132, 0xFF47DD82, 0xFF4ACBDC, 0xFF4E4E4E, 0xFF000000, 0xFF000000,
    0xFFFEFFFF, 0xFFC0DEFF, 0xFFD2D1FF, 0xFFE7C7FF, 0xFFF8C2FF, 0xFFFFC3E9, 0xFFFFCBC4, 0xFFF5D7A5, 0xFFE2E394, 0xFFCEED96, 0xFFBCF2AA, 0xFFB3F1CB, 0xFFB4E9F0, 0xFFB6B6B6, 0xFF000000, 0xFF000000,
};

uint32_t nesPalette[64] = {
    0xFF656565, 0xFF002A84, 0xFF1513A2, 0xFF3A019E, 0xFF59007A, 0xFF6A003E, 0xFF680800, 0xFF531D00, 0xFF323400, 0xFF0D4600, 0xFF004F00, 0xFF004C09, 0xFF003F4B, 0xFF000000, 0xFF000000, 0xFF000000,
    0xFFAEAEAE, 0xFF175FD6, 0xFF4341FF, 0xFF7529FA, 0xFF9E1DCA, 0xFFB4207B, 0xFFB13322, 0xFF964E00, 0xFF6A6C00, 0xFF398400, 0xFF0F9000, 0xFF008D33, 0xFF007B8C, 0xFF000000, 0xFF000000, 0xFF000000,
    0xFFFEFFFF, 0xFF66AFFF, 0xFF9390FF, 0xFFC578FF, 0xFFEE6CFF, 0xFFFF6FCA, 0xFFFF8271, 0xFFE69E25, 0xFFBABC00, 0xFF88D501, 0xFF5EE132, 0xFF47DD82, 0xFF4ACBDC, 0xFF4E4E4E, 0xFF000000, 0xFF000000,
    0xFFFEFFFF, 0xFFC0DEFF, 0xFFD2D1FF, 0xFFE7C7FF, 0xFFF8C2FF, 0xFFFFC3E9, 0xFFFFCBC4, 0xFFF5D7A5, 0xFFE2E394, 0xFFCEED96, 0xFFBCF2AA, 0xFFB3F1CB, 0xFFB4E9F0, 0xFFB6B6B6, 0xFF000000, 0xFF000000,
};

//ppu implementation that is incredibly fucked

uint8_t PPU::readCHR(uint16_t addr) {
    return globalROM.mapper ? globalROM.mapper->ppuRead(addr) : addr;
}

void PPU::Render(SDL_Renderer* renderer) {
    uint32_t pixels[NES_WIDTH * NES_HEIGHT];
    uint8_t palOffset = 4;

    if (UseRandPalIndex)
        palOffset = RanPalIndex;;

    if (PaletteMode == 1) {
        for (int i = 0; i < 64; i++) {
            uint8_t r = (nesPalette[i] >> 16) & 0xFF;
            uint8_t g = (nesPalette[i] >> 8) & 0xFF;
            uint8_t b = (nesPalette[i] >> 0) & 0xFF;
            r = static_cast<uint8_t>(r * 0.95f);
            g = static_cast<uint8_t>(g * 0.95f);
            b = static_cast<uint8_t>(b * 0.98f);
            nesPalette[i] = (r << 16) | (g << 8) | b;
        }
    }

    for (int screenY = 0; screenY < NES_HEIGHT; screenY++) {
        for (int screenX = 0; screenX < NES_WIDTH; screenX++) {
            int absX = screenX + scrollX;
            int absY = screenY + scrollY;
            int ntH = (absX / NES_WIDTH) & 1;
            int ntV = (absY / NES_HEIGHT) & 1;
            int nametable = nametableSelect ^ ntH ^ (ntV << 1);
            int nametableBase = 0x2000 | (nametable << 10);

            int tileX = (absX / 8) & 31;
            int tileY = (absY / 8) % 30;
            int fineX = absX % 8;
            int fineY = absY % 8;

            int vramAddr = (nametableBase + tileY * 32 + tileX) & 0x0FFF;
            uint8_t tileIndex = VRAM[vramAddr];

            uint16_t vaddr = (BGPatternTable ? 0x1000 : 0x0000) + tileIndex * 16 + fineY;
            uint8_t lo = readCHR(vaddr);
            uint8_t hi = readCHR(vaddr + 8);

            int attrX = tileX / 4;
            int attrY = tileY / 4;
            int attrAddr = (nametableBase + 0x3C0 + attrY * 8 + attrX) & 0x0FFF;
            uint8_t attrByte = VRAM[attrAddr];
            int quadrantX = (tileX % 4) / 2;
            int quadrantY = (tileY % 4) / 2;
            int attrShift = (quadrantY * 2 + quadrantX) * 2;
            uint8_t paletteIndex = (attrByte >> attrShift) & 0x03;

            int bit = 7 - fineX;
            int colorBits = ((hi >> bit) & 1) << 1 | ((lo >> bit) & 1);
            uint8_t finalPal = colorBits ? paletteRAM[colorBits + paletteIndex * palOffset] : paletteRAM[0];
            uint32_t color = nesPalette[finalPal & 0x3F];

            pixels[screenY * NES_WIDTH + screenX] = color;
        }
    }
    if (!DisableSprites || !maskRenderSprites) {
        for (int i = 0; i < MaxSprites; i++) {
            int spriteY = OAM[i * 4 + 0] + 1;
            int tile = OAM[i * 4 + 1];
            int attr = OAM[i * 4 + 2];
            int spriteX = OAM[i * 4 + 3];

            bool flipH = attr & 0x40;
            bool flipV = attr & 0x80;
            uint8_t paletteIndex = attr & 0x03;

            int spriteHeight = use8x16Sprites ? 16 : 8;

            uint16_t vaddrBase;
            if (use8x16Sprites) {
                uint16_t patternTable = (tile & 0x01) ? 0x1000 : 0x0000;
                int tileIndex = (tile & 0xFE);
                vaddrBase = patternTable + tileIndex * 16;
            } else {
                vaddrBase = (spritePatternTable ? 0x1000 : 0x0000) + tile * 16;
            }

            for (int row = 0; row < spriteHeight; row++) {
                int tileRow = row;
                uint16_t vaddr = vaddrBase;

                if (use8x16Sprites) {
                    if ((!flipV && row >= 8) || (flipV && row < 8)) {
                        vaddr += 16;
                    }
                    int inTileRow = flipV ? 7 - (row & 7) : (row & 7);
                    vaddr += inTileRow;
                } else {
                    int inTileRow = flipV ? 7 - row : row;
                    vaddr += inTileRow;
                }

                uint8_t plane0 = readCHR(vaddr);
                uint8_t plane1 = readCHR(vaddr + 8);

                for (int col = 0; col < 8; col++) {
                    int tileCol = flipH ? 7 - col : col;
                    uint8_t colorLow  = (plane0 >> (7 - tileCol)) & 1;
                    uint8_t colorHigh = (plane1 >> (7 - tileCol)) & 1;
                    uint8_t colorId = (colorHigh << 1) | colorLow;
                    if (colorId == 0) continue;

                    int px = spriteX + col;
                    int py = spriteY + row;
                    if (px < 0 || px >= NES_WIDTH || py < 0 || py >= NES_HEIGHT) continue;

                    uint8_t palEntry = paletteRAM[(palOffset * 4) + (paletteIndex * 4) + colorId] & 0x3F;
                    pixels[py * NES_WIDTH + px] = nesPalette[palEntry];
                }
            }
        }
    }

    SDL_UpdateTexture(texture, nullptr, pixels, NES_WIDTH * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    // SDL_RenderPresent(renderer);
}
