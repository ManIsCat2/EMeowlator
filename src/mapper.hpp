#pragma once
#include <stdint.h>

class NesROM;
class CPU;
class PPU;

class Mapper {
public:
    int prgBankOffset[4] = {0, 0x4000, 0, 0};
    int ChrBankOffset[8] = {0, 0x1000, 0, 0, 0, 0, 0, 0};
    virtual ~Mapper() = default;
    virtual uint8_t cpuRead(uint16_t addr) { (void)addr; return 0; }
    virtual void cpuWrite(uint16_t addr, uint8_t value) { (void)addr; (void)value; }
    virtual uint8_t ppuRead(uint16_t addr) { (void)addr; return 0; }
    virtual const char *getName(void) { return ""; }
    virtual void reset() {}
};
