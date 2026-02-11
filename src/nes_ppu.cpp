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

uint8_t PPU::readCHR(uint16_t addr) {
    addr &= 0x1FFF;

    if (addr < 0x1000) {
        return ChrData[ ChrBankOffset[0] + addr ];
    } else {
        return ChrData[ ChrBankOffset[1] + (addr - 0x1000) ];
    }
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
    0xFF757575,0xFF271B8F,0xFF0000AB,0xFF47009F,0xFF8F0077,0xFFAB0013,0xFFA70000,0xFF7F0B00,
    0xFF432F00,0xFF004700,0xFF005100,0xFF003F17,0xFF1B3F5F,0xFF000000,0xFF000000,0xFF000000,
    0xFFBCBCBC,0xFF0073EF,0xFF233BEF,0xFF8300F3,0xFFBF00BF,0xFFE7005B,0xFFDB2B00,0xFFCB4F0F,
    0xFF8B7300,0xFF009F0F,0xFF00AB00,0xFF00933B,0xFF00838B,0xFF000000,0xFF000000,0xFF000000,
    0xFFFFFFFF,0xFF3FBFFF,0xFF5F97FF,0xFFA78BFD,0xFFF77BFF,0xFFFF77B7,0xFFFF7763,0xFFFF9B3B,
    0xFFF3BF3F,0xFF83D313,0xFF4FDF4B,0xFF58F898,0xFF00EBDB,0xFF505050,0xFF000000,0xFF000000,
    0xFFFFFFFF,0xFFA7E7FF,0xFFC7D7FF,0xFFD7CBFF,0xFFFFC7FF,0xFFFFC7DB,0xFFFFBFB3,0xFFFFDBAB,
    0xFFFFE7A3,0xFFE3FFA3,0xFFABF3BF,0xFFB3FFCF,0xFF9FFFF3,0xFF000000,0xFF000000,0xFF000000
};

uint32_t nesPalette[64] = {
    0xFF757575,0xFF271B8F,0xFF0000AB,0xFF47009F,0xFF8F0077,0xFFAB0013,0xFFA70000,0xFF7F0B00,
    0xFF432F00,0xFF004700,0xFF005100,0xFF003F17,0xFF1B3F5F,0xFF000000,0xFF000000,0xFF000000,
    0xFFBCBCBC,0xFF0073EF,0xFF233BEF,0xFF8300F3,0xFFBF00BF,0xFFE7005B,0xFFDB2B00,0xFFCB4F0F,
    0xFF8B7300,0xFF009F0F,0xFF00AB00,0xFF00933B,0xFF00838B,0xFF000000,0xFF000000,0xFF000000,
    0xFFFFFFFF,0xFF3FBFFF,0xFF5F97FF,0xFFA78BFD,0xFFF77BFF,0xFFFF77B7,0xFFFF7763,0xFFFF9B3B,
    0xFFF3BF3F,0xFF83D313,0xFF4FDF4B,0xFF58F898,0xFF00EBDB,0xFF505050,0xFF000000,0xFF000000,
    0xFFFFFFFF,0xFFA7E7FF,0xFFC7D7FF,0xFFD7CBFF,0xFFFFC7FF,0xFFFFC7DB,0xFFFFBFB3,0xFFFFDBAB,
    0xFFFFE7A3,0xFFE3FFA3,0xFFABF3BF,0xFFB3FFCF,0xFF9FFFF3,0xFF000000,0xFF000000,0xFF000000
};

//ppu implementation that is incredibly fucked

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
            uint16_t vaddrBase = (spritePatternTable ? 0x1000 : 0x0000) + tile * 16;

            for (int row = 0; row < 8; row++) {
                int tileRow = flipV ? 7 - row : row;

                uint8_t plane0 = readCHR(vaddrBase + tileRow);
                uint8_t plane1 = readCHR(vaddrBase + tileRow + 8);

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
