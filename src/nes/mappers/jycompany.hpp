#pragma once
#include "mapper_base.hpp"
#include <stdint.h>

class JyCompany : public MapperBase {
public:
    JyCompany();
    ~JyCompany() override = default;

    uint8_t cpuRead(uint16_t addr) override;
    void cpuWrite(uint16_t addr, uint8_t value) override;
    const char* getName(void) override;
    void reset() override;

    uint16_t getCHRPageSize() override {
        return 0x400;
    }
    uint16_t getPRGPageSize() override {
        return 0x2000;
    }
private:
    uint8_t prgRegs[4];
    uint8_t prgMode;
    bool enablePrgAt6000;

    uint8_t chrLowRegs[8];
    uint8_t chrHighRegs[8];
    uint8_t chrLatch[2];
    uint8_t chrMode;
    bool advancedNtControl;
    bool chrBlockMode;
    uint8_t chrBlock;
    bool mirrorChr;

    uint8_t mirroringReg;

    bool irqEnabled;
    uint8_t irqSource; 
    uint8_t irqCounter;
    uint8_t irqPrescaler;
    uint8_t irqXorReg;
    uint8_t irqFunkyModeReg;
    uint16_t lastPpuAddr;

    uint8_t multiplyValue1;
    uint8_t multiplyValue2;
    uint8_t regRamValue;

    void updatePrg();
    void updateCHR();
    void updateMirroring();
    void updateState();
    uint16_t getChrReg(int index);
};