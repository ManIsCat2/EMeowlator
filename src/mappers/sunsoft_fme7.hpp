#pragma once
#include "mapper_base.hpp"
#include <stdint.h>

class SunSoftFME7 : public MapperBase {
public:
    SunSoftFME7();
    ~SunSoftFME7() override = default;

    uint8_t cpuRead(uint16_t addr) override;
    uint8_t ppuRead(uint16_t addr) override;
    void cpuWrite(uint16_t addr, uint8_t value) override;
    void reset() override;
    const char* getName(void) override;

    void clockIRQ();
private:
    uint8_t command;
    uint8_t workRamValue;

    bool irqEnabled;
    bool irqCounterEnabled;
    uint16_t irqCounter;

    uint8_t chrRegs[8];
    uint8_t prgRegs[3];

    void updateChr();
    void updatePrg();
};