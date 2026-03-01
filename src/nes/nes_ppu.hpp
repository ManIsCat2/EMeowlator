#pragma once
#include <array>
#include <cstdint>
#include <vector>
#include <cstring>

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

    MirrorMode Mirroring = MirrorMode::VERTICAL;
    bool WriteLatch = false;
    unsigned short TransferAddr = 0;
    unsigned short VRAMAddr = 0;
    unsigned short OAMAddr = 0;
    unsigned short TempVRAMAddr = 0;
    uint8_t ReadBuffer = 0;
    int Dot = 0;
    int ScanLine = 0;
    bool Vblank = false;
    bool sprite0Hit = false;

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

    uint16_t scrollX = 0;
    uint16_t scrollY = 0;
    uint8_t scrollFineX = 0;

    bool DisableSprites = false;
    int MaxSprites = 64;
    bool VRAMCorruption = false;
    bool DisableXScroll = false;
    bool DisableYScroll = false;

    void reset(void) {
        memset(VRAM.data(), 0, VRAM.size());
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
        spritePatternTable = false;
        BGPatternTable = false;
        use8x16Sprites = false;
        enableNMI = false;

        scrollX = 0;
        scrollY = 0;
        scrollFineX = 0;
    }

    void Step();

    void LoadCHRROM(const uint8_t* chrData, int chrSize);
    uint8_t readCHR(uint16_t addr);

    bool Init();
};

extern PPU ppu;
extern uint32_t nesPalette[64];
extern uint32_t nesPaletteDefault[64];
extern float rainbowHoverPhase;