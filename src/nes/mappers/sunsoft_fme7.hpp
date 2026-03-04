#pragma once
#include "mapper_base.hpp"
#include <stdint.h>

class SunSoftFME7 : public MapperBase {
public:
    SunSoftFME7();
    ~SunSoftFME7() override = default;

    uint8_t cpuRead(uint16_t addr) override;
    void cpuWrite(uint16_t addr, uint8_t value) override;
    void reset() override;
    const char* getName(void) override;

    uint16_t getCHRPageSize() override {
        return 0x400;
    }
    uint16_t getPRGPageSize() override {
        return 0x2000;
    }

    void clockCPU(void) override;
private:
    uint8_t command;
    uint8_t workRamValue;

    bool irqEnabled;
    bool irqCounterEnabled;
    uint16_t irqCounter; 
};