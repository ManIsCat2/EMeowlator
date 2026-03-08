#pragma once

#include <stdint.h>
#include <string>

class VFilterBase {
public:
    virtual ~VFilterBase() {};
    virtual void initialize(void) {};
    virtual void applyFilter(void) {};
};

#include "../nes_ppu.hpp"