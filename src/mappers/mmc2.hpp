#pragma once
#include "mapper_base.hpp"
#include <stdint.h>

class MMC2 : public MapperBase {
public:
    MMC2();
    ~MMC2() override = default;

    uint8_t cpuRead(uint16_t addr) override;
    uint8_t ppuRead(uint16_t addr) override;
    void cpuWrite(uint16_t addr, uint8_t value) override;
    const char *getName(void) override;
    void reset() override;

    uint16_t getCHRSlotSize() override {
        return 0x1000;
    }
private:
    uint8_t PrgBank;
    bool ChrUpdate;

    uint8_t ChrBankFD[2];
    uint8_t ChrBankFE[2];

    uint8_t Latch[2];

    void updatePRG();
};
