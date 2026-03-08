#pragma once
#include "filter_base.hpp"

class ChromaFilter : public VFilterBase {
public:
    ~ChromaFilter() override = default;
    void applyFilter(void) override;;
};