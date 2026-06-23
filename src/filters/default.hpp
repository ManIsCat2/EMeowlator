#pragma once
#include "filter_base.hpp"

class DefaultFilter : public VFilterBase {
public:
    void applyFilter(UNUSED uint32_t *pix, UNUSED int x, UNUSED int y) override;
};