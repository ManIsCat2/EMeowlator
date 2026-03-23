#pragma once
#include "filter_base.hpp"

class ChromaFilter : public VFilterBase {
public:
    void applyFilter(uint32_t *pix, int x, int y) override;
};