#pragma once
#include "mbc_base.hpp"

class MBC3 : public MBCBase {
public:
    MBC3();

    const char* getName(void) override;
    void reset() override;
    
    uint8_t cpuRead(uint16_t addr) override;
    void cpuWrite(uint16_t addr, uint8_t value) override;
private:
    bool ramEnable = false;
    uint8_t romBank = 1;
    uint8_t ramBank = 0;

    void updateBanks();
};