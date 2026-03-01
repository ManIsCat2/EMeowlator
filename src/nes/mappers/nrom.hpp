#pragma once
#include "mapper_base.hpp"
#include <stdint.h>

class NROM : public MapperBase {
public:
    NROM();
    ~NROM() override = default;

    const char *getName(void) override;
    void reset() override;
};
