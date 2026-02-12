#pragma once
#include <stdint.h>

class NesROM;
class CPU;
class PPU;

class MapperBase {
public:
    int PRGBankOffset[4] = {0, 0, 0, 0};
    int CHRBankOffset[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    virtual ~MapperBase() = default;
    virtual uint8_t cpuRead(uint16_t addr) { (void)addr; return 0; }
    virtual uint8_t cpuReadAfter0x8000(uint16_t addr) { (void)addr; return 0; }
    virtual void cpuWrite(uint16_t addr, uint8_t value) { (void)addr; (void)value; }
    virtual uint8_t ppuRead(uint16_t addr) { (void)addr; return 0; }
    virtual const char *getName(void) { return ""; }
    virtual void reset() {}
};
