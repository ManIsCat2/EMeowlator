#pragma once

#include <cstdint>
#include <QImage>
#include "gb_bus.hpp"

class GbPPU : public HasGBBus {
public:
    GbPPU();
    ~GbPPU();

    uint8_t VRAM[8192];
    uint8_t OAM[160];

    uint8_t LCDC = 0x00;
    uint8_t STAT = 0x00;
    uint8_t SCY  = 0x00;
    uint8_t SCX  = 0x00;
    uint8_t LY   = 0x00;
    uint8_t LYC  = 0x00;
    uint8_t DMA  = 0x00;
    uint8_t BGP  = 0xFC;
    uint8_t OBP0 = 0xFF;
    uint8_t OBP1 = 0xFF;
    uint8_t WY   = 0x00;
    uint8_t WX   = 0x00;

    int scanlineCounter = 0;
    uint32_t frameBuffer[160 * 144];
    QImage *rawOutputImage = nullptr;

    void reset();
    void Step(uint8_t cycles);
    void RenderScanline();
    
    uint8_t readVRAM(uint16_t addr);
    void writeVRAM(uint16_t addr, uint8_t value);
    uint8_t readOAM(uint16_t addr);
    void writeOAM(uint16_t addr, uint8_t value);
    uint8_t readRegister(uint16_t addr);
    void writeRegister(uint16_t addr, uint8_t value);
};

extern GbPPU gbPpu;