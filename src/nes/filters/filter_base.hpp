#pragma once

#include <nes.hpp>

#include <stdint.h>
#include <string>

class VFilterBase {
public:
    virtual ~VFilterBase() {};
    virtual void initialize(void) {};
    virtual void applyFilter(UNUSED uint32_t *pix, UNUSED int x, UNUSED int y) {};
    virtual bool hasCustomBlit(void) { return false; };
    virtual void blit(void) {};
};

#include "../nes_ppu.hpp"