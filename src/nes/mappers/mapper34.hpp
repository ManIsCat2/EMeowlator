#pragma once
#include "mapper_base.hpp"
#include <stdint.h>

class Mapper34 : public MapperBase {
public:
    Mapper34(bool isNina01=false);

    void cpuWrite(uint16_t addr, uint8_t value) override;
    const char *getName(void) override;
    void reset() override;

    void saveState(SaveStateFile &s) override {
        s.WriteBytes<uint8_t>(PRGBank0);
        if (nina01) {
            s.WriteBytes<uint8_t>(CHRBank0);
            s.WriteBytes<uint8_t>(CHRBank1);
        }
    }
    void loadState(SaveStateFile &s) override {
        PRGBank0 = s.ReadBytes<uint8_t>();
        if (nina01) {
            CHRBank0 = s.ReadBytes<uint8_t>();
            CHRBank1 = s.ReadBytes<uint8_t>();
            setCHRBank(0, CHRBank0);
            setCHRBank(1, CHRBank1);
        }
        setPRGBank(0, PRGBank0);
    }

    uint16_t getCHRBankSize() override {
        return nina01 ? 0x1000 : 0x2000;
    }
    uint16_t getPRGBankSize() override {
        return 0x8000;
    }
private:
    bool nina01;
    uint8_t PRGBank0, CHRBank0, CHRBank1 = 0;
};
