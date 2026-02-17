#include "bnrom.hpp"
#include "../nes_cpu.hpp"
#include "../nes_ppu.hpp"

BNROM::BNROM() {
    reset();
}

void BNROM::reset() {
    setPRGSlot(0, 0);
}

void BNROM::cpuWrite(uint16_t addr, uint8_t value) {
    if (addr >= 0x8000) {
        setPRGSlot(0, value);
        return;
    }
    MapperBase::cpuWrite(addr, value);
}

const char* BNROM::getName(void) {
    return "BNROM";
}