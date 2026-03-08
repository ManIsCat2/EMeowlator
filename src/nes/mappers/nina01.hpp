#pragma once
#include "mapper_base.hpp"
#include <stdint.h>

class NINA01 : public MapperBase {
public:
    NINA01();

    void cpuWrite(uint16_t addr, uint8_t value) override;
    const char *getName(void) override;
    void reset() override;

    uint16_t getCHRPageSize() override {
        return 0x1000;
    }
    uint16_t getPRGPageSize() override {
        return 0x8000;
    }
};
