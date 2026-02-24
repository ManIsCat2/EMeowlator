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

void PPU::Step() {
    if (Dot == 1 && ScanLine == 241) Vblank = true;
    if (Dot == 1 && ScanLine == 261) {
        Vblank = false;
        sprite0Hit = false;
    } 

    if (ScanLine >= 0 && ScanLine < NES_HEIGHT && Dot >= 1 && Dot <= NES_WIDTH) {
        int x = Dot - 1;
        int y = ScanLine;
        uint32_t color = 0;
        uint8_t bgColorId = 0;

        if (maskRenderBG) {
            int absX = x + (DisableXScroll ? 0 : scrollX);
            int absY = y + (DisableYScroll ? 0 : scrollY);
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
            uint8_t finalPal = colorBits ? paletteRAM[colorBits + paletteIndex * 4] : paletteRAM[0];
            bgColorId = finalPal & 0x3F;
            color = nesPalette[bgColorId];
            if ((bgColorId) == hoveredPaletteIndex) color = getRainbowColor();
        }

        if (maskRenderSprites && !DisableSprites) {
            bool spritePixelDrawn = false;
            for (int i = 0; i < MaxSprites; i++) {
                int spriteY = OAM[i * 4 + 0] + 1;
                int tile = OAM[i * 4 + 1];
                int attr = OAM[i * 4 + 2];
                int spriteX = OAM[i * 4 + 3];

                int spriteHeight = use8x16Sprites ? 16 : 8;
                if (y < spriteY || y >= spriteY + spriteHeight) continue;

                bool flipH = attr & 0x40;
                bool flipV = attr & 0x80;
                uint8_t paletteIndex = attr & 0x03;

                if (x < spriteX || x >= spriteX + 8) continue;

                uint16_t patternTable;
                uint16_t vaddrBase;

                int rowInSprite = y - spriteY;

                if (use8x16Sprites) {
                    patternTable = (tile & 0x01) ? 0x1000 : 0x0000;
                    int tileIndex = tile & 0xFE;
                    if (rowInSprite < 8) {
                        vaddrBase = patternTable + tileIndex * 16;
                    } else {
                        vaddrBase = patternTable + (tileIndex + 1) * 16;
                    }
                    int inTileRow = rowInSprite & 7;
                    if (flipV) inTileRow = 7 - inTileRow;
                    vaddrBase += inTileRow;
                } else {
                    patternTable = spritePatternTable ? 0x1000 : 0x0000;
                    vaddrBase = patternTable + tile * 16;
                    int inTileRow = flipV ? 7 - rowInSprite : rowInSprite;
                    vaddrBase += inTileRow;
                }

                uint8_t plane0 = readCHR(vaddrBase);
                uint8_t plane1 = readCHR(vaddrBase + 8);

                int colInTile = x - spriteX;
                int tileCol = flipH ? 7 - colInTile : colInTile;
                uint8_t colorLow  = (plane0 >> (7 - tileCol)) & 1;
                uint8_t colorHigh = (plane1 >> (7 - tileCol)) & 1;
                uint8_t colorId = (colorHigh << 1) | colorLow;

                if (colorId == 0) continue;
                if (!sprite0Hit && i == 0 && bgColorId != 0) {
                    sprite0Hit = true;
                }

                if (!spritePixelDrawn) {
                    uint8_t palEntry = paletteRAM[16 + paletteIndex * 4 + colorId] & 0x3F;
                    color = nesPalette[palEntry];
                    if (palEntry == hoveredPaletteIndex) color = getRainbowColor();
                    spritePixelDrawn = true;
                }
            }
        }
        frameBuffer[y * NES_WIDTH + x] = color;
    }

    Dot++;
    if (Dot > 341) {
        Dot = 0;
        ScanLine++;
        if (ScanLine > 261) ScanLine = 0;
    }
}

uint8_t PPU::readCHR(uint16_t addr) {
    return globalROM.mapper ? globalROM.mapper->ppuRead(addr) : addr;
}

void PPU::LoadCHRROM(const uint8_t* chrData, int chrSize) {
    ChrData.resize(chrSize);
    memcpy(ChrData.data(), chrData, chrSize);
}

bool PPU::Init() {
    PaletteMode = 0;
    
    memset(frameBuffer, 0, sizeof(frameBuffer));
    return true;
}