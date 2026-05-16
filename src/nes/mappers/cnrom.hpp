#pragma once
#include "mapper_base.hpp"
#include <stdint.h>

class CNROM : public MapperBase {
public:
    CNROM();

    void cpuWrite(uint16_t addr, uint8_t value) override;
    const char *getName(void) override;
    void reset() override;

    void saveState(SaveStateFile &s) override {
        s.WriteBytes<uint8_t>(CHRBank);
    }
    void loadState(SaveStateFile &s) override {
        CHRBank = s.ReadBytes<uint8_t>();
        setCHRPages(0, CHRBank);
    }

    uint16_t getCHRPageSize() override {
        return 0x2000;
    }
    uint16_t getPRGPageSize() override {
        return 0x8000;
    }
private:
    uint8_t CHRBank = 0;
};
