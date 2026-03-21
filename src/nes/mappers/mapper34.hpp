#pragma once
#include "mapper_base.hpp"
#include <stdint.h>

class Mapper34 : public MapperBase {
public:
    Mapper34(bool isNina01=false);

    void cpuWrite(uint16_t addr, uint8_t value) override;
    const char *getName(void) override;
    void reset() override;

    uint16_t getCHRPageSize() override {
        return nina01 ? 0x1000 : 0x2000;
    }
    uint16_t getPRGPageSize() override {
        return 0x8000;
    }
private:
    bool nina01;
};
