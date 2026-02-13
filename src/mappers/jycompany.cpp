#include "jycompany.hpp"
#include "../nes_cpu.hpp"
#include "../nes_ppu.hpp"
#include <cstring>

JyCompany::JyCompany() {
    reset();
}

void JyCompany::reset() {
    memset(prgRegs, 0, sizeof(prgRegs));
    memset(chrLowRegs, 0, sizeof(chrLowRegs));
    memset(chrHighRegs, 0, sizeof(chrHighRegs));
    chrLatch[0] = 0;
    chrLatch[1] = 4;

    prgMode = 0;
    enablePrgAt6000 = false;

    chrMode = 0;
    chrBlockMode = false;
    chrBlock = 0;
    mirrorChr = false;
    mirroringReg = 0;

    irqEnabled = false;
    irqSource = 0;
    irqCounter = 0;
    irqPrescaler = 0;
    irqXorReg = 0;
    irqFunkyModeReg = 0;
    lastPpuAddr = 0;

    multiplyValue1 = 0;
    multiplyValue2 = 0;
    regRamValue = 0;

    updatePrg();
    updateChr();
    updateMirroring();
}

uint8_t JyCompany::cpuRead(uint16_t addr) {
    if(addr < 0x8000) {
        switch(addr & 0xF803) {
            case 0x5800: return 0x19; // this makes shit work in smw bootleg idk why
            case 0x5801: return multiplyValue2;
            case 0x5803: return regRamValue;
        }
        if (addr >= 0x6000 && enablePrgAt6000) {
            return cpu.PrgRAM[addr - 0x6000];
        }
    } else {
        int bank = (addr - 0x8000) >> 13;
        return globalROM.ROM[PRGBankOffset[bank] + (addr & 0x1FFF)];
    }
    return 0xff;
}

void JyCompany::cpuWrite(uint16_t addr, uint8_t value) {
    if(addr < 0x8000) {
        switch(addr & 0xF803) {
            case 0x5800: multiplyValue1 = value; break;
            case 0x5801: multiplyValue2 = value; break;
            case 0x5803: regRamValue = value; break;
        }
        if (addr >= 0x6000 && enablePrgAt6000) {
            cpu.PrgRAM[addr - 0x6000] = value;
        }
    } else {
        switch(addr & 0xF007) {
            case 0x8000: case 0x8001: case 0x8002: case 0x8003:
                prgRegs[addr & 0x03] = value & 0x7F;
                updatePrg();
                break;
            case 0x9000: case 0x9001: case 0x9002: case 0x9003:
            case 0x9004: case 0x9005: case 0x9006: case 0x9007:
                chrLowRegs[addr & 0x07] = value;
                updateChr();
                break;
            case 0xA000: case 0xA001: case 0xA002: case 0xA003:
            case 0xA004: case 0xA005: case 0xA006: case 0xA007:
                chrHighRegs[addr & 0x07] = value;
                updateChr();
                break;
            case 0xC000: irqEnabled = value & 0x01; if(!irqEnabled) cpu.doIRQ = false; break;
            case 0xC004: irqPrescaler = value ^ irqXorReg; break;
            case 0xC005: irqCounter = value ^ irqXorReg; break;
            case 0xC006: irqXorReg = value; break;
            case 0xC007: irqFunkyModeReg = value; break;
            case 0xD000:
                prgMode = value & 0x07;
                chrMode = (value >> 3) & 0x03;
                enablePrgAt6000 = (value & 0x80) != 0;
                updatePrg();
                updateChr();
                break;
            case 0xD001:
                mirroringReg = value & 0x03;
                updateMirroring();
                break;

            case 0xD003:
                mirrorChr = (value & 0x80) != 0;
                chrBlockMode = (value & 0x20) == 0;
                chrBlock = ((value & 0x18) >> 2) | (value & 0x01);
                updateChr();
                break;
        }
    }
}

const char* JyCompany::getName() {
    return "JyCompany ASIC";
}

void JyCompany::updatePrg() {
    size_t prgCount = globalROM.PRGRomSize / 0x2000;

    switch(prgMode & 0x03) {
        case 0:
            PRGBankOffset[0] = 0; 
            PRGBankOffset[1] = 1 * 0x2000;
            PRGBankOffset[2] = 2 * 0x2000;
            PRGBankOffset[3] = (prgCount - 1) * 0x2000;
            break;
        case 1:
            PRGBankOffset[0] = prgRegs[0] * 0x2000;
            PRGBankOffset[1] = prgRegs[1] * 0x2000;
            PRGBankOffset[2] = prgRegs[2] * 0x2000;
            PRGBankOffset[3] = (prgCount - 1) * 0x2000;
            break;
        case 2:
        case 3:
            for(int i=0;i<3;i++) PRGBankOffset[i] = prgRegs[i]*0x2000;
            PRGBankOffset[3] = (prgCount - 1) * 0x2000;
            break;
    }
    if(enablePrgAt6000) PRGBankOffset[0] = prgRegs[0]*0x2000;
}

uint16_t JyCompany::getChrReg(int index) {
    if(chrBlockMode) {
        return (chrLowRegs[index] & 0x1F) | (chrBlock << 5);
    } else {
        return chrLowRegs[index] | (chrHighRegs[index] << 8);
    }
}

void JyCompany::updateChr() {
    for(int i=0;i<8;i++)
        CHRBankOffset[i] = getChrReg(i)*0x400;
}

void JyCompany::updateMirroring() {
    switch(mirroringReg) {
        case 0: ppu.Mirroring = 0; break;
        case 1: ppu.Mirroring = 1; break;
        case 2: ppu.Mirroring = 2; break;
        case 3: ppu.Mirroring = 3; break;
    }
}

uint8_t JyCompany::ppuRead(uint16_t addr) {
    addr &= 0x1FFF;
    int bank = addr >> 10;
    return ppu.ChrData[CHRBankOffset[bank] + (addr & 0x3FF)];
}