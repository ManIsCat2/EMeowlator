#pragma once
#include "mapper.hpp"
#include <stdint.h>

class CNROM : public Mapper {
public:
    CNROM();
    ~CNROM() override = default;

    uint8_t cpuRead(uint16_t addr) override;
    uint8_t cpuReadAfter0x8000(uint16_t addr) override;
    uint8_t ppuRead(uint16_t addr) override;
    void cpuWrite(uint16_t addr, uint8_t value) override;
    const char *getName(void) override;
    void reset() override;

private:
    uint8_t ChrBank;
    void updateBanks();
};
