#pragma once
#include <array>
#include <cstdint>
#include <vector>
#include <cstring>

#include "nes.hpp"
#include "nes_ntsc.h"
#include "filters/filters.hpp"
#include "mappers/mapper_base.hpp"

enum class MirrorMode {
    HORIZONTAL,
    VERTICAL,
    SCREEN_A,
    SCREEN_B,
    FOURSCREEN
};

enum class VideoFilter {
    NONE,
    NTSC,
    CHROMA,
    GRAYSCALE
};

class PPU {
public:
    PPU();
    ~PPU();
    std::vector<uint8_t> ChrData{};
    std::array<uint8_t, VRAM_SIZE> VRAM{};
    std::array<uint8_t, PALRAM_SIZE> paletteRAM{};
    uint8_t OAM[0x100];
    uint32_t *frameBuffer = nullptr;
    uint8_t *palIndexBuf = nullptr;

    VFilterBase *vfilter = nullptr;
    VideoFilter filtering = VideoFilter::NONE;

    MirrorMode Mirroring = MirrorMode::VERTICAL;
    int busDecayTimers[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    // random numbers i wrote :pray:
    const uint8_t busDecayMasks[8] = {0xff, 0xf9, 0xf1, 0xe9, 0x8d, 0xf3, 0x32, 0xfe};
    uint8_t dataBus = 0;
    bool WriteLatch = false;
    uint16_t VRAMAddr = 0;
    uint16_t OAMAddr = 0;
    uint16_t TransferAddr = 0;
    uint8_t ReadBuffer = 0;
    int Dot = 0;
    int ScanLine = 0;
    bool Vblank = false;
    bool sprite0Hit = false;
    bool spriteOverflow = false;

    struct {
        bool grayscaleMode = false;
        bool background8pxMask = false;
        bool sprite8pxMask = false;
        bool renderBackground = false;
        bool renderSprites = false;
        uint8_t combined = 0;
    } mask;

    struct {
        int nametableSelect = 0; 
        bool VRAMInc32 = false;
        int spritePatternTable = 0;
        int BGPatternTable = 0;
        bool use8x16Sprites = false;
        bool enableNMI = false;
        uint8_t combined = 0;
    } control;

    bool DisableSprites = false;
    bool VRAMCorruption = false;
    uint8_t scrollFineX = 0;
    uint8_t patternTableLow = 0;
    uint8_t patternTableHigh = 0;
    uint8_t nametableByte = 0;
    uint16_t shiftRegHigh = 0;
    uint16_t shiftRegLow = 0;
    uint16_t shiftAttrHigh = 0;
    uint16_t shiftAttrLow = 0;
    uint16_t attributeByte = 0;

    MapperBase* romMapper = nullptr;

    void reset(void);
    void resetBusDecay(void);
    void decayDataBus(void);
    uint16_t getAttributeByte();
    void RenderScreen(void);
    void Step();

    void LoadCHRROM(const uint8_t* chrData, int chrSize);
    uint16_t mirrorNametable(uint16_t addr);
    void blitPixels();

    void Init();
    VFilterBase *GetVideoFilter(VideoFilter filter);
    void InitFilter(VideoFilter filter);
};

extern PPU ppu;
extern uint32_t nesPalette[64];
extern uint32_t nesPaletteDefault[64];
extern float rainbowHoverPhase;