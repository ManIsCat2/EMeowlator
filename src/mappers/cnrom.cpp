#include "cnrom.hpp"
#include "../nes_cpu.hpp"
#include "../nes_ppu.hpp"

CNROM::CNROM() {
    reset();
}

void CNROM::reset() {
    setPRGSlot(0, 0);
}

void CNROM::cpuWrite(uint16_t addr, uint8_t value) {
    if (addr >= 0x8000) {
        setCHRSlot(0, value);
        setCHRSlot(1, value, 0x1000); // without this, star force breaks???
        return;
    }
    MapperBase::cpuWrite(addr, value);
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