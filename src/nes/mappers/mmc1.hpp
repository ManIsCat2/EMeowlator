#pragma once
#include "mapper_base.hpp"
#include <stdint.h>

class MMC1 : public MapperBase {
public:
    MMC1();

    void cpuWrite(uint16_t addr, uint8_t value) override;
    const char *getName(void) override;
    void reset() override;

    void saveState(SaveStateFile &s) override {
        s.WriteBytes<uint8_t>(WriteBuffer);
        s.WriteBytes<uint8_t>(shiftCount);

        s.WriteBytes<bool>(ChrMode);
        s.WriteBytes<bool>(PrgMode);
        s.WriteBytes<bool>(slotSelect);

        s.WriteBytes<uint8_t>(control);

        s.WriteBytes<uint8_t>(chrReg0);
        s.WriteBytes<uint8_t>(chrReg1);
        s.WriteBytes<uint8_t>(prgReg);
        
        s.WriteBytes<int>((int)ppu.Mirroring);
    }
    void loadState(SaveStateFile &s) override {
        WriteBuffer = s.ReadBytes<uint8_t>();
        shiftCount = s.ReadBytes<uint8_t>();

        ChrMode = s.ReadBytes<bool>();
        PrgMode = s.ReadBytes<bool>();
        slotSelect = s.ReadBytes<bool>();

        control = s.ReadBytes<uint8_t>();

        chrReg0 = s.ReadBytes<uint8_t>();
        chrReg1 = s.ReadBytes<uint8_t>();
        prgReg = s.ReadBytes<uint8_t>();

        ppu.Mirroring = (MirrorMode)s.ReadBytes<int>();

        updateBanks();
    }

    uint16_t getCHRPageSize() override {
        return 0x1000;
    }
    uint16_t getPRGPageSize() override {
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
