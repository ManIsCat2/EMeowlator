#pragma once
#include "mapper_base.hpp"
#include <stdint.h>

class UxROM : public MapperBase {
public:
    UxROM();
    ~UxROM() override = default;

    uint8_t ppuRead(uint16_t addr) override;
    void cpuWrite(uint16_t addr, uint8_t value) override;
    const char *getName(void) override;
    void reset() override;

    uint16_t getCHRSlotSize() override {
        return 0x2000;
    }
    uint16_t getPRGSlotSize() override {
        return 0x4000;
    }
};
