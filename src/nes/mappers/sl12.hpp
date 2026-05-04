#pragma once
#include "mapper_base.hpp"
#include <stdint.h>

class SL12 : public MapperBase {
public:
    SL12();

    void cpuWrite(uint16_t addr, uint8_t value) override;
    const char *getName(void) override;
    void reset() override;

    uint16_t getCHRPageSize() override {
        return 0x400;
    }
    uint16_t getPRGPageSize() override {
        return 0x2000;
    }
    void clockPPU(void) override;
private:
    uint8_t mode = 0;

	uint8_t MMC3Regs[10] = {};
	uint8_t MMC3Ctrl = 0;
	uint8_t MMC3Mirroring = 0;

	uint8_t VRC2Chr[8] = {};
	uint8_t VRC2Prg[2] = {};
	uint8_t VRC2Mirroring = 0;

	uint8_t MMC1Regs[4] = {};
	uint8_t MMC1Buffer = 0;
	uint8_t MMC1Shift = 0;

	uint8_t IRQCounter = 0;
	uint8_t IRQReloadVal = 0;
	bool IRQReload = false;
	bool IRQEnable = false;

    void updatePRG(void);
    void updateCHR(void);
    void updateMirroring(void);
    void update(void);
    void writeVRC2(uint16_t addr, uint8_t value);
    void writeMMC3(uint16_t addr, uint8_t value);
    void writeMMC1(uint16_t addr, uint8_t value);
};