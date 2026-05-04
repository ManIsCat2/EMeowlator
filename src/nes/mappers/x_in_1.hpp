#pragma once
#include "mapper_base.hpp"
#include <stdint.h>

class XIn1 : public MapperBase {
public:
    XIn1();

    void cpuWrite(uint16_t addr, uint8_t value) override;
    const char *getName(void) override;
    void reset() override;

    uint16_t getCHRPageSize() override {
        return 0x2000;
    }
    uint16_t getPRGPageSize() override {
        return 0x4000;
    }
};