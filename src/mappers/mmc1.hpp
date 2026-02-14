#pragma once
#include "mapper_base.hpp"
#include <stdint.h>

class MMC1 : public MapperBase {
public:
    MMC1();
    ~MMC1() override = default;

    uint8_t cpuRead(uint16_t addr) override;
    uint8_t ppuRead(uint16_t addr) override;
    void cpuWrite(uint16_t addr, uint8_t value) override;
    const char *getName(void) override;
    void reset() override;

    uint16_t getCHRSlotSize() override {
        return 0x1000;
    }
    uint16_t getPRGSlotSize() override {
        return 0x4000;
    }
private:
    uint8_t ShiftReg;
    uint8_t ShiftCount;
    uint8_t Ctrl;
    uint8_t ChrBank0;
    uint8_t ChrBank1;
    uint8_t PrgBank;

    int PrgMode;
    int ChrMode;

    void modifyRegister(uint16_t addr, uint8_t data);
    void updateBanks();
};
