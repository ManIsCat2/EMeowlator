#pragma once
#include "filter_base.hpp"
#include "../nes_ntsc.h"

class NTSCFilter : public VFilterBase {
public:
    void initialize(void) override;
    void applyFilter(void) override;
private:
    nes_ntsc_t NTSC;
    nes_ntsc_setup_t NTSCSetup;
};