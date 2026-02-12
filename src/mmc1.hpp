#pragma once
#include "mapper.hpp"
#include <stdint.h>

class MMC1 : public Mapper {
public:
    MMC1();
    ~MMC1() override = default;

    uint8_t cpuRead(uint16_t addr) override;
    uint8_t ppuRead(uint16_t addr) override;
    void cpuWrite(uint16_t addr, uint8_t value) override;
    const char *getName(void) override;
    void reset() override;


private:
    uint8_t ShiftReg;
    uint8_t ShiftCount;
    uint8_t Ctrl;
    uint8_t ChrBank0;
    uint8_t ChrBank1;
    uint8_t PrgBank;
    int ChrBankOffset[2] = {0, 0x1000};

    int PrgMode;
    int ChrMode;

    void modifyRegister(uint16_t addr, uint8_t data);
    void updateBanks();
};
