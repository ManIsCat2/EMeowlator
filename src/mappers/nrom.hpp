#pragma once
#include "mapper_base.hpp"
#include <stdint.h>

class NROM : public MapperBase {
public:
    NROM();
    ~NROM() override = default;

    uint8_t ppuRead(uint16_t addr) override;
    const char *getName(void) override;
    void reset() override;
};
