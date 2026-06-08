#pragma once
#include "mapper_base.hpp"
#include <stdint.h>
#include "../nes_ppu.hpp"

class NROM : public MapperBase {
public:
    NROM();

    const char *getName(void) override;
    void reset() override;
};
