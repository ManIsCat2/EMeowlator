#pragma once
#include "mapper_base.hpp"
#include <stdint.h>

class MMC2 : public MapperBase {
public:
    MMC2();
    ~MMC2() override = default;

    uint8_t ppuRead(uint16_t addr) override;
    void cpuWrite(uint16_t addr, uint8_t value) override;
    const char *getName(void) override;
    void reset() override;

    uint16_t getCHRPageSize() override {
        return 0x1000;
    }
    uint16_t getPRGPageSize() override {
        return 0x2000;
    }
private:
    bool ChrUpdate;

    uint8_t ChrBankFD[2];
    uint8_t ChrBankFE[2];

    uint8_t Latch[2];
};
