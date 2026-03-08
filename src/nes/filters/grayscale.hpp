#pragma once
#include "filter_base.hpp"

class GrayScaleFilter : public VFilterBase {
public:
    void applyFilter(void) override;
};