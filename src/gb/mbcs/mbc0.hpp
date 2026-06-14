#pragma once
#include "mbc_base.hpp"

class MBC0 : public MBCBase {
public:
    MBC0();

    const char *getName(void) override;
    void reset() override;
};
