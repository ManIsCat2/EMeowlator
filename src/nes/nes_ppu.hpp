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
    uint8_t OAM[0x100];
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
    uint8_t FullPPUCTRL = 0;

    int nametableSelect = 0; 
    bool VRAMInc32Mode = false;
    int spritePatternTable = 0;
    int BGPatternTable = 0;
    bool use8x16Sprites = false;
    bool enableNMI = false;

    uint8_t scrollFineX = 0;

    uint8_t patternTableLow = 0;
    uint8_t patternTableHigh = 0;
    uint8_t ntb = 0;
    uint16_t shiftRegHigh = 0;
    uint16_t shiftRegLow = 0;
    uint16_t attributeByte = 0;
    int shiftAttribute = 0;

    bool DisableSprites = false;
    int MaxSprites = 64;
    bool VRAMCorruption = false;

    void reset(void);

    void Step();

    void LoadCHRROM(const uint8_t* chrData, int chrSize);
    uint8_t readCHR(uint16_t addr);
    uint8_t readVRAM(uint16_t addr);

    bool Init();
};

extern PPU ppu;
extern uint32_t nesPalette[64];
extern uint32_t nesPaletteDefault[64];
extern float rainbowHoverPhase;