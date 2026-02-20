#pragma once
#include <array>
#include <cstdint>
#include <vector>

#include "nes.hpp"

enum class MirrorMode {
    HORIZONTAL,
    VERTICAL,
    SCREEN_A,
    SCREEN_B,
    FOURSCREEN
};

class PPU {
public:
    std::vector<uint8_t> ChrData{};
    std::array<uint8_t, VRAM_MIRRORED_SIZE> VRAM{};
    std::array<uint8_t, PALRAM_SIZE> paletteRAM{};
    std::array<uint8_t, 0x100> OAM{};
    uint32_t frameBuffer[NES_WIDTH * NES_HEIGHT];

    MirrorMode Mirroring;
    bool WriteLatch = false;
    unsigned short TransferAddr = 0;
    unsigned short VRAMAddr = 0;
    unsigned short OAMAddr = 0;
    unsigned short TempVRAMAddr = 0;
    uint8_t ReadBuffer = 0;
    int Dot = 0;
    int ScanLine = 0;
    bool Vblank = false;

    bool mask8pxMaskBG = false;
    bool mask8pxMaskSprites = false;
    bool maskRenderBG = false;
    bool maskRenderSprites = false;

    int nametableSelect = 0; 
    bool VRAMInc32Mode = false;
    bool spritePatternTable = false;
    bool BGPatternTable = false;
    bool use8x16Sprites = false;
    bool enableNMI = false;

    uint16_t scrollX = 0;  // horizontal scroll in pixels
    uint16_t scrollY = 0;  // vertical scroll in pixels
    uint8_t scrollFineX = 0; // 0-7, fine pixel shift inside a tile

    int PaletteMode = 0;
    bool UseRandPalIndex = false;
    uint8_t RanPalIndex = 4;
    bool DisableSprites = false;
    int MaxSprites = 64;
    bool VRAMCorruption = false;
    bool DisableXScroll = false;
    bool DisableYScroll = false;

    void Step();

    void LoadCHRROM(const uint8_t* chrData, int chrSize);
    uint8_t readCHR(uint16_t addr);

    bool Init();
    void Render();
};

extern PPU ppu;
extern uint32_t nesPalette[64];
extern uint32_t nesPaletteDefault[64];