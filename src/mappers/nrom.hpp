#pragma once
#include "mapper_base.hpp"
#include <stdint.h>

class NROM : public MapperBase {
public:
    NROM();
    ~NROM() override = default;

    uint8_t cpuRead(uint16_t addr) override;
    uint8_t ppuRead(uint16_t addr) override;
    void cpuWrite(uint16_t addr, uint8_t value) override;
    const char *getName(void) override;
    void reset() override;
};
