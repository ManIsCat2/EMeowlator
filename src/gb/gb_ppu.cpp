#include "gb_ppu.hpp"
#include "gb_cpu.hpp"
#include <cstring>

GbPPU gbPpu;

uint32_t gbPaletteDefault[4] = {
    0xFFE0F8D0,
    0xFF88C070,
    0xFF346856,
    0xFF081820
};

uint32_t gbPalette[4] = {
    0xFFE0F8D0,
    0xFF88C070,
    0xFF346856,
    0xFF081820
};

GbPPU::GbPPU() {
    initFilter(VideoFilter::NONE);
    rawOutputImage = new QImage((uint8_t*)frameBuffer, 160, 144, QImage::Format_RGB32);
    filteredOutputImage = new QImage((uint8_t*)(frameBuffer), NES_NTSC_OUT_WIDTH(160), 144, QImage::Format_RGB32);
}

GbPPU::~GbPPU() {
    delete rawOutputImage;
    delete filteredOutputImage;
    delete vfilter;
}

void GbPPU::reset() {
    connectBus(&gbCpu, nullptr);

    memset(VRAM, 0, sizeof(VRAM));
    memset(OAM, 0, sizeof(OAM));
    memset(frameBuffer, 0, sizeof(frameBuffer));
    memset(palIndexBuf, 0, sizeof(palIndexBuf));

    LCDC = 0x91;
    STAT = 0x85;
    SCY  = 0x00;
    SCX  = 0x00;
    LY   = 0x00;
    LYC  = 0x00;
    DMA  = 0xFF;
    BGP  = 0xFC;
    OBP0 = 0xFF;
    OBP1 = 0xFF;
    WY   = 0x00;
    WX   = 0x00;

    scanlineCounter = 456;
}

void GbPPU::Step(uint8_t cycles) {
    if (!(LCDC & 0x80)) {
        LY = 0;
        scanlineCounter = 456;
        STAT = (STAT & ~0x03);
        return;
    }

    scanlineCounter -= cycles;

    while (scanlineCounter <= 0) {
        scanlineCounter += 456;
        LY++;

        if (LY == 144) {
            cpu->IF |= 0x01;

            if (vfilter->hasCustomBlit()) {
                vfilter->blit();
            } else {
                blitPixels();
            }
        }

        if (LY > 153) {
            LY = 0;
        }

        if (LY == LYC) {
            STAT |= 0x04;

            if (STAT & 0x40) {
                cpu->IF |= 0x02;
            }
        } else {
            STAT &= ~0x04;
        }
    }

    uint8_t oldMode = STAT & 0x03;
    uint8_t newMode;

    if (LY >= 144) {
        newMode = 1;
    } else if (scanlineCounter >= 376) {
        newMode = 2;
    } else if (scanlineCounter >= 204) {
        newMode = 3;
    } else {
        newMode = 0;
    }

    if (newMode != oldMode) {
        if (oldMode == 3 && newMode == 0) {
            RenderScanline();
        }

        switch (newMode) {
            case 0:
                if (STAT & 0x08) cpu->IF |= 0x02;
                break;

            case 1:
                if (STAT & 0x10) cpu->IF |= 0x02;
                break;

            case 2:
                if (STAT & 0x20) cpu->IF |= 0x02;
                break;
        }

        STAT = (STAT & ~0x03) | newMode;
    }
}

void GbPPU::RenderScanline() {
    uint16_t tileMap = (LCDC & 0x08) ? 0x9C00 : 0x9800;
    uint16_t tileData = (LCDC & 0x10) ? 0x8000 : 0x8800;
    bool isUnsigned = (LCDC & 0x10);

    uint16_t yPos = (uint16_t)(SCY + LY);
    uint16_t tileRow = ((yPos / 8) % 32) * 32;

    uint8_t bgLineColorIds[160] = {0};

    if (LCDC & 0x01) {
        for (int pixel = 0; pixel < 160; pixel++) {
            uint8_t xPos = pixel + SCX;
            uint16_t tileCol = (xPos / 8);
            uint16_t tileAddress = tileMap + tileRow + tileCol;

            uint16_t tileDataLocation;
            if (isUnsigned) {
                uint8_t tileNum = VRAM[tileAddress - 0x8000];
                tileDataLocation = tileData + (tileNum * 16);
            } else {
                int8_t tileNum = (int8_t)VRAM[tileAddress - 0x8000];
                int16_t offset = tileNum * 16;
                tileDataLocation = 0x9000 + offset;
            }
            
            uint8_t line = (yPos % 8) * 2;
            uint8_t byte1 = VRAM[tileDataLocation + line - 0x8000];
            uint8_t byte2 = VRAM[tileDataLocation + line + 1 - 0x8000];

            int bitBit = 7 - (xPos % 8);
            int colorBit0 = (byte1 >> bitBit) & 0x01;
            int colorBit1 = (byte2 >> bitBit) & 0x01;
            uint8_t colorId = (colorBit1 << 1) | colorBit0;
            
            bgLineColorIds[pixel] = colorId;

            uint8_t colorPaletteShade = (BGP >> (colorId * 2)) & 0x03;
            palIndexBuf[LY * 160 + pixel] = colorPaletteShade;
        }
    } else {
        for(int pixel = 0; pixel < 160; pixel++) {
            palIndexBuf[LY * 160 + pixel] = 0;
        }
    }

    if ((LCDC & 0x20) && LY >= WY) {
        uint16_t windowTileMap = (LCDC & 0x40) ? 0x9C00 : 0x9800;
        int windowY = LY - WY;
        int tileRow = ((windowY / 8) & 31) * 32;

        for (int pixel = 0; pixel < 160; pixel++) {
            int windowX = pixel - (WX - 7);
            if (windowX < 0) continue;

            uint16_t tileCol = (windowX / 8) & 31;
            uint16_t tileAddr = windowTileMap + tileRow + tileCol;
            uint16_t tileDataLocation = 0;

            if (isUnsigned) {
                uint8_t tileNum = VRAM[tileAddr - 0x8000];
                tileDataLocation = 0x8000 + tileNum * 16;
            } else {
                int8_t tileNum = (int8_t)VRAM[tileAddr - 0x8000];
                tileDataLocation = 0x9000 + tileNum * 16;
            }

            uint8_t line = (windowY % 8) * 2;
            uint8_t byte1 = VRAM[tileDataLocation + line - 0x8000];
            uint8_t byte2 = VRAM[tileDataLocation + line + 1 - 0x8000];

            int bit = 7 - (windowX % 8);

            uint8_t colorId = (((byte2 >> bit) & 1) << 1) | ((byte1 >> bit) & 1);
            bgLineColorIds[pixel] = colorId;

            uint8_t shade = (BGP >> (colorId * 2)) & 3;
            palIndexBuf[LY * 160 + pixel] = shade;
        }
    }

    if (!(LCDC & 0x02)) return;

    bool use8x16 = (LCDC & 0x04);
    int spriteHeight = use8x16 ? 16 : 8;
    int spritesFound = 0;

    for (int i = 0; i < 40; i++) {
        uint16_t oamBase = i * 4;
        
        int spriteY = OAM[oamBase] - 16;
        int spriteX = OAM[oamBase + 1] - 8;
        uint8_t tileNum = OAM[oamBase + 2];
        uint8_t attributes = OAM[oamBase + 3];

        if (use8x16) {
            tileNum &= 0xFE; 
        }

        if (LY >= spriteY && LY < (spriteY + spriteHeight)) {
            spritesFound++;
            if (spritesFound > 10) break;

            bool objToBgPriority = (attributes & 0x80); 
            bool yFlip           = (attributes & 0x40); 
            bool xFlip           = (attributes & 0x20); 
            uint8_t paletteReg   = (attributes & 0x10) ? OBP1 : OBP0; 

            int lineInsideSprite = LY - spriteY;
            if (yFlip) {
                lineInsideSprite = spriteHeight - 1 - lineInsideSprite;
            }

            uint16_t tileDataLocation = 0x8000 + (tileNum * 16) + (lineInsideSprite * 2);
            uint8_t byte1 = VRAM[tileDataLocation - 0x8000];
            uint8_t byte2 = VRAM[tileDataLocation + 1 - 0x8000];

            for (int tilePixel = 0; tilePixel < 8; tilePixel++) {
                int pixelX = spriteX + tilePixel;
                
                if (pixelX < 0 || pixelX >= 160) continue;

                int bitBit = xFlip ? tilePixel : (7 - tilePixel);
                int colorBit0 = (byte1 >> bitBit) & 0x01;
                int colorBit1 = (byte2 >> bitBit) & 0x01;
                uint8_t colorId = (colorBit1 << 1) | colorBit0;

                if (colorId == 0) continue;

                if (objToBgPriority && bgLineColorIds[pixelX] != 0) {
                    continue; 
                }

                uint8_t colorPaletteShade = (paletteReg >> (colorId * 2)) & 0x03;
                if (!DisableSprites) {
                    palIndexBuf[LY * 160 + pixelX] = colorPaletteShade;
                }
            }
        }
    }
}

void GbPPU::blitPixels() {
    for (int y = 0; y < 144; y++) {
        for (int x = 0; x < 160; x++) {
            int i = y * 160 + x;
            frameBuffer[i] = gbPalette[palIndexBuf[i]];
            vfilter->applyFilter(&frameBuffer[i], x, y);
        }
    }
}

uint8_t GbPPU::readVRAM(uint16_t addr) {
    return VRAM[addr - 0x8000];
}
void GbPPU::writeVRAM(uint16_t addr, uint8_t value) {
    uint16_t offset = addr - 0x8000; 

    if (VRAMCorruption && (rand() & 7) == 0) {
        offset ^= (1 << (rand() % 13)); 
    }

    offset &= 0x1FFF; 
    VRAM[offset] = value;
}
uint8_t GbPPU::readOAM(uint16_t addr) {
    return OAM[addr - 0xFE00];
}
void GbPPU::writeOAM(uint16_t addr, uint8_t value) {
    OAM[addr - 0xFE00] = value;
}

uint8_t GbPPU::readRegister(uint16_t addr) {
    switch (addr) {
        case 0xFF40: return LCDC;
        case 0xFF41: return STAT | 0x80;
        case 0xFF42: return SCY;
        case 0xFF43: return SCX;
        case 0xFF44: return LY;
        case 0xFF45: return LYC;
        case 0xFF46: return DMA;
        case 0xFF47: return BGP;
        case 0xFF48: return OBP0;
        case 0xFF49: return OBP1;
        case 0xFF4A: return WY;
        case 0xFF4B: return WX;
        default:     return 0xFF;
    }
}

void GbPPU::writeRegister(uint16_t addr, uint8_t value) {
    switch (addr) {
        case 0xFF40: {
            LCDC = value; 
            if (!(value & 0x80)) {
                LY = 0;
                scanlineCounter = 456;
                STAT = (STAT & ~0x03);
            }
            break;
        }
        case 0xFF41:
            STAT = (STAT & 0x07) | (value & 0x78);
            break;
        case 0xFF42: SCY = value; break;
        case 0xFF43: SCX = value; break;
        case 0xFF44:
            LY = 0;
            if (LY == LYC) {
                STAT |= 0x04;
            } else {
                STAT &= ~0x04;
            }
            break;
        case 0xFF45:
            LYC = value;
            if (LY == LYC) {
                STAT |= 0x04;
                if (STAT & 0x40) cpu->IF |= 0x02;
            } else {
                STAT &= ~0x04;
            }

            break;
        case 0xFF46: {
            DMA = value;

            uint16_t src = value << 8;
            for (int i = 0; i < 160; i++) {
                OAM[i] = cpu->read(src + i);
            }
            break;
        }
        case 0xFF47: BGP = value; break;
        case 0xFF48: OBP0 = value; break;
        case 0xFF49: OBP1 = value; break;
        case 0xFF4A: WY = value; break;
        case 0xFF4B: WX = value; break;
    }
}