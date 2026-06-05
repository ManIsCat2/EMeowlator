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

PPU::PPU() {
    frameBuffer = new uint32_t[PPU_PIXEL_COUNT_NTSC];
    palIndexBuf = new uint8_t[PPU_PIXEL_COUNT];
    rawOutputImage = new QImage((uint8_t*)(frameBuffer), NES_WIDTH, NES_HEIGHT, QImage::Format_RGB32);
    filteredOutputImage = new QImage((uint8_t*)(frameBuffer), NES_NTSC_OUT_WIDTH(256), NES_HEIGHT, QImage::Format_RGB32);
}

PPU::~PPU() {
    delete[] frameBuffer;
    delete[] palIndexBuf;
    delete rawOutputImage;
    delete filteredOutputImage;
    delete vfilter;
}

void PPU::reset(void) {
    memset(VRAM.data(), 0, VRAM_SIZE);
    memset(OAM, 0, 0x100);
    memset(paletteRAM.data(), 0, PALRAM_SIZE);
    memset(frameBuffer, 0, sizeof(uint32_t) * PPU_PIXEL_COUNT_NTSC);
    memset(palIndexBuf, 0, PPU_PIXEL_COUNT);
    dataBus = 0;
    for (int i = 0; i < 8; i++) { 
        busDecayTimers[i] = 0;
    }
    WriteLatch = false;
    VRAMAddr = 0;
    OAMAddr = 0;
    TransferAddr = 0;
    ReadBuffer = 0;
    Dot = 0;
    ScanLine = 0;
    Vblank = false;
    sprite0Hit = false;
    
    mask.background8pxMask = false;
    mask.sprite8pxMask = false;
    mask.renderBackground = false;
    mask.renderSprites = false;
    
    control.nametableSelect = 0; 
    control.VRAMInc32 = false;
    control.spritePatternTable = 0;
    control.BGPatternTable = 0;
    control.use8x16Sprites = false;
    control.enableNMI = false;
    
    scrollFineX = 0;
}

void PPU::resetBusDecayTimers(void) {
    for (int i = 0; i < 8; i++) { 
        busDecayTimers[i] = PPU_BUS_DECAY_TIME;
    }
}

void PPU::decayDataBus(void) {
    for (int i = 0; i < 8; i++) {
        if (busDecayTimers[i] != 0) {
            busDecayTimers[i]--;
            if (busDecayTimers[i] == 0) {
                dataBus &= busDecayMasks[i];
            }
        }
    }
}

uint16_t PPU::getAttributeByte() {
    if (globalROM.mapper->usingExtendedAttributes()) {
        MMC5 *mmc5 = (MMC5*)globalROM.mapper;
        uint8_t ex = mmc5->getEXRAMByte(VRAMAddr);
        return (ex >> 6) & 0x03;
        
    }
    uint16_t attrAddr = (VRAMAddr & 0x0C00) | 0x03C0 | ((VRAMAddr >> 4) & 0x38) | ((VRAMAddr >> 2) & 0x07);
    uint8_t attr = globalROM.mapper->readVRAM(attrAddr);
    uint8_t shift = ((VRAMAddr >> 4) & 4) | (VRAMAddr & 2);
    
    return (attr >> shift) & 0x03;
}

void PPU::RenderScreen() {
    if ((uint32_t)(Dot - NES_WIDTH) <= 63) return;

    bool renderBG = mask.renderBackground;
    bool renderSPR = mask.renderSprites;

    if (Dot < NES_WIDTH) {
        uint8_t bgColor = 0;
        uint8_t bgPalette = 0;

        if (renderBG && (Dot >= 8 || mask.background8pxMask)) {
            uint16_t bit = 0x8000 >> scrollFineX;
            bgColor = ((shiftRegHigh & bit) ? 2 : 0) | ((shiftRegLow  & bit) ? 1 : 0);
            bgPalette = ((((shiftAttrHigh & bit) ? 2 : 0) | ((shiftAttrLow  & bit) ? 1 : 0)) << 2);
        }

        uint8_t finalColor = bgColor;
        uint8_t finalPalette = bgPalette;

        if (renderSPR) {
            uint16_t spriteH = control.use8x16Sprites ? 16 : 8;
            for (int i = 0; i < 256; i += 4) {
                uint8_t *sprite = &OAM[i];
                uint16_t spriteX = Dot - sprite[3];
                uint16_t spriteY = ScanLine - sprite[0] - 1;
                
                int flipH = sprite[2] & 0x40;
                int flipV = sprite[2] & 0x80;
                
                uint16_t sx = flipH ? spriteX : (7 - spriteX);
                uint16_t sy = flipV ? (spriteH - 1 - spriteY) : spriteY;
                if (spriteX < 8 && spriteY < spriteH) {
                    uint16_t spriteTile = sprite[1];
                    uint16_t spriteAddress = (control.use8x16Sprites ? spriteTile % 0x02 << 0x0C | spriteTile << 4 & -32 | sy * 0x02 & 0x10 : (control.spritePatternTable) << 0x09 | spriteTile << 0x04) | sy & 0x07;
                    uint16_t spriteColor = globalROM.mapper->readCHR(spriteAddress + 8, true) >> sx << 0x01 & 0x02 | globalROM.mapper->readCHR(spriteAddress, true) >> sx & 0x01;
                    if (spriteColor) {
                        if (!(sprite[2] & 0x20 && bgColor) && !DisableSprites) {
                            finalColor = spriteColor;
                            finalPalette = 0x10 | sprite[2] * 0x04 & 0x0C;
                        }
                            
                        if (i == 0 && bgColor && Dot != 255 && renderBG && (Dot >= 8 || mask.sprite8pxMask)) {
                            sprite0Hit = true;
                        }
                        break;
                    }
                }
            }
        }

        uint8_t palIndex = paletteRAM[finalColor ? (finalPalette | finalColor) : 0] & 0x3F;
        if (palIndex == hoveredPaletteIndex) palIndex = 254;
        palIndexBuf[ScanLine * NES_WIDTH + Dot] = palIndex;
    }

    if (Dot < 336) {
        shiftRegHigh <<= 1;
        shiftRegLow <<= 1;
        shiftAttrLow <<= 1;
        shiftAttrHigh <<= 1;
    }

    uint16_t fetchAddress = ((control.BGPatternTable) ? 0x1000 : 0x0000) | (nametableByte << 4) | ((VRAMAddr >> 12) & 7);
    switch ((Dot + 1) & 7) {
        case 0: {
            shiftRegLow = (shiftRegLow & 0xFF00) | patternTableLow;
            shiftRegHigh = (shiftRegHigh & 0xFF00) | patternTableHigh;
            shiftAttrLow = (shiftAttrLow & 0xFF00) | ((attributeByte & 1) ? 0xFF : 0x00);
            shiftAttrHigh = (shiftAttrHigh & 0xFF00) | ((attributeByte & 2) ? 0xFF : 0x00);

            if ((VRAMAddr & VRAM_COX) == VRAM_COX) {
                VRAMAddr &= ~VRAM_COX;
                VRAMAddr ^= VRAM_X_NT;
            } else {
                VRAMAddr++;
            }
            break;
        }

        case 1:
            nametableByte = globalROM.mapper->readVRAM(VRAMAddr);
            break;

        case 3: {
            attributeByte = getAttributeByte();
            break;
        }

        case 5:
            patternTableLow = globalROM.mapper->readCHR(fetchAddress, false);
            break;

        case 7:
            patternTableHigh = globalROM.mapper->readCHR(fetchAddress + 8, false);
            break;
    }
}

void PPU::Step() {
    const bool isPal = /*globalROM.Region == ConsoleRegion::PAL*/ false;
    const int preRenderLine = isPal ? 311 : 261;
    const int totalScanlines = isPal ? 312 : 262;

    if (Dot == 1 && ScanLine == 241) Vblank = true;
    if (Dot == 0 && ScanLine == 241) {
        if (vfilter->hasCustomBlit()) {
            vfilter->blit();
        } else {
            blitPixels();
        }
    }

    if (Dot == 1 && ScanLine == preRenderLine) {
        Vblank = false;
        sprite0Hit = false;
    }

    if (mask.renderBackground || mask.renderSprites) {
        if (ScanLine < 240) {
            RenderScreen();

            if (Dot == 256) {
                if ((VRAMAddr & VRAM_FIY) != VRAM_FIY) {
                    VRAMAddr += 0x1000;
                } else {
                    VRAMAddr &= 0x0FFF;
                    int y = (VRAMAddr & VRAM_COY) >> 5;
                    if (y == 29) {
                        y = 0;
                        VRAMAddr ^= VRAM_Y_NT;
                    } else if (y == 31) {
                        y = 0;
                    } else {
                        y++;
                        y &= 0x1F;
                    }
                    VRAMAddr = ((VRAMAddr & 0xFC1F) | (y << 5));
                }
            }

            if (Dot == 257) {
                VRAMAddr = ((VRAMAddr & 0x7be0) | (TransferAddr & 0x41f));
            }
        }

        if (Dot >= 280 && Dot <= 304 && ScanLine == preRenderLine) {
            VRAMAddr = ((VRAMAddr & 0x41f) | (TransferAddr & 0x7be0));
        }
        globalROM.mapper->clockPPU();
    }

    Dot++;
    if (Dot >= 341) {
        Dot = 0;
        ScanLine++;
        if (ScanLine >= totalScanlines) ScanLine = 0;
    }
    if (dataBus != 0) decayDataBus();
}

void PPU::LoadCHRROM(const uint8_t *data, int chrSize) {
    ChrData.resize(chrSize);
    memcpy(ChrData.data(), data, chrSize);
}

uint16_t PPU::mirrorNametable(uint16_t addr) {
    uint16_t mirrored = addr;
    switch (Mirroring) {
        case MirrorMode::HORIZONTAL:
            mirrored = (addr & 0x3FF) | ((addr & 0x800) >> 1);
            break;
        case MirrorMode::VERTICAL:
            mirrored = addr & 0x7FF;
            break;
        case MirrorMode::SCREEN_A:
            mirrored = addr & 0x3FF;
            break;
        case MirrorMode::SCREEN_B:
            mirrored = (addr & 0x3FF) | 0x400;
            break;
        case MirrorMode::FOURSCREEN: {
            uint16_t nt = (addr >> 10) & 0x03;
            mirrored = (nt << 10) | (addr & 0x3FF);
            break;
        }
    }
    return mirrored;
}

void PPU::blitPixels() {
    for (int y = 0; y < NES_HEIGHT; y++) {
        for (int x = 0; x < NES_WIDTH; x++) {
            int i = y * NES_WIDTH + x;
            frameBuffer[i] = nesPalette[palIndexBuf[i] & 0x3f];
            vfilter->applyFilter(&frameBuffer[i], x, y);
            if (palIndexBuf[i] == 254) frameBuffer[i] = getRainbowColor();
        }
    }
}

void PPU::Init() {
    InitFilter(VideoFilter::NONE);
    memset(frameBuffer, 0, sizeof(uint32_t) * PPU_PIXEL_COUNT_NTSC);
    memset(palIndexBuf, 0, PPU_PIXEL_COUNT);
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