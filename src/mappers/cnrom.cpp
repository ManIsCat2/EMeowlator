#include "cnrom.hpp"
#include "../nes_cpu.hpp"
#include "../nes_ppu.hpp"

CNROM::CNROM() {
    reset();
}

void CNROM::reset() {
    ChrBank = 0;
    CHRBankOffset[0] = 0;
    CHRBankOffset[1] = 0x1000;
}

uint8_t CNROM::cpuRead(uint16_t addr) {
    if (addr < 0x8000) {
        if (addr >= 0x6000) {
            return cpu.PrgRAM[addr - 0x6000];
        }
    } else {
        return globalROM.ROM[addr - 0x8000];
    }
    return 0xff;
}

void CNROM::cpuWrite(uint16_t addr, uint8_t value) {
    if (addr >= 0x8000) {
        ChrBank = value & 0x03;
        updateBanks();
    }
}

const char* CNROM::getName(void) {
    return "CNROM";
}

uint8_t CNROM::ppuRead(uint16_t addr) {
    addr &= 0x1FFF;
    if (addr < 0x1000) {
        return ppu.ChrData[CHRBankOffset[0] + addr];
    } else {
        return ppu.ChrData[CHRBankOffset[1] + (addr - 0x1000)];
    }
}

void CNROM::updateBanks() {
    size_t base = (size_t)ChrBank * 0x2000;
    base %= globalROM.CHRRomSize;

    CHRBankOffset[0] = base;
    CHRBankOffset[1] = base + 0x1000;
}
