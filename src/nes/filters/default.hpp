#pragma once
#include "filter_base.hpp"

class DefaultFilter : public VFilterBase {
public:
    ~DefaultFilter() override = default;
    void applyFilter(void) override;
};