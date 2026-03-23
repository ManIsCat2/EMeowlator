#pragma once
#include "filter_base.hpp"

class GrayScaleFilter : public VFilterBase {
public:
    void applyFilter(uint32_t *pix, UNUSED int x, UNUSED int y) override;
};