#pragma once
#include "mapper_base.hpp"
#include <stdint.h>

class MMC3 : public MapperBase {
public:
    MMC3();

    void cpuWrite(uint16_t addr, uint8_t value) override;
    const char *getName(void) override;
    void reset() override;

    uint16_t getCHRPageSize() override {
        return 0x400;
    }
    uint16_t getPRGPageSize() override {
        return 0x2000;
    }
    uint16_t getSRAMSize() override {
        return subMapper == 1 ? 0x400 : 0x2000;
    }

    void clockPPU(void) override;
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
};