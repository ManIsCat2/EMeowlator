#pragma once
#include "mapper_base.hpp"
#include <stdint.h>

class MMC1 : public MapperBase {
public:
    MMC1();
    ~MMC1() override = default;

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
    uint8_t WriteBuffer = 0;
	uint8_t shiftCount = 0;

	bool ChrMode = false;
	bool PrgMode = false;
	bool slotSelect = false;
    uint8_t control = 0;

	uint8_t chrReg0 = 0;
	uint8_t chrReg1 = 0;
	uint8_t prgReg = 0;

    void modifyRegister(uint16_t addr, uint8_t data);
    void updateBanks();
};
