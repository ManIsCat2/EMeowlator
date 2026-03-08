#pragma once
#include "filter_base.hpp"

class DefaultFilter : public VFilterBase {
public:
    void applyFilter(void) override;
};