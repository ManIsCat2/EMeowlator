#pragma once
#include "mapper_base.hpp"
#include <stdint.h>

class MMC3 : public MapperBase {
public:
    MMC3();
    ~MMC3() override = default;

    uint8_t cpuRead(uint16_t addr) override;
    uint8_t ppuRead(uint16_t addr) override;
    void cpuWrite(uint16_t addr, uint8_t value) override;
    const char *getName(void) override;
    void reset() override;

    uint16_t getCHRSlotSize() override {
        return 0x400;
    }
    uint16_t getPRGSlotSize() override {
        return 0x2000;
    }
private:
    uint8_t BankSelect;
    uint8_t BankRegisters[8];

    uint8_t PrgMode;
    uint8_t ChrMode;

    uint8_t IRQReload;
    uint8_t IRQCounter;
    bool IRQEnabled;
    bool LastA12;

    void updatePRG();
    void updateCHR();
    void clockIRQ(bool a12);
};