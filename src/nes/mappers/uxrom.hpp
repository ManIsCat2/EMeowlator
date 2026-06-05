#pragma once
#include "mapper_base.hpp"
#include <stdint.h>

class UxROM : public MapperBase {
public:
    UxROM();

    void cpuWrite(uint16_t addr, uint8_t value) override;
    const char *getName(void) override;
    void reset() override;

    void saveState(SaveStateFile &s) override {
        s.WriteBytes<uint8_t>(PRGBank);
    }
    void loadState(SaveStateFile &s) override {
        PRGBank = s.ReadBytes<uint8_t>();
        setCHRBank(0, PRGBank);
    }

    uint16_t getCHRBankSize() override {
        return 0x2000;
    }
    uint16_t getPRGBankSize() override {
        return 0x4000;
    }
private:
    uint8_t PRGBank = 0;
};
