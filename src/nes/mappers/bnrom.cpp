#include "bnrom.hpp"
#include "../nes_cpu.hpp"
#include "../nes_ppu.hpp"

BNROM::BNROM() {
    reset();
}

void BNROM::reset() {
    setPRGPage(0, 0);
    setCHRPage(0, 0);
}

void BNROM::cpuWrite(uint16_t addr, uint8_t value) {
    if (addr >= 0x8000) {
        setPRGPage(0, value);
        return;
    }
    MapperBase::cpuWrite(addr, value);
}

const char* BNROM::getName(void) {
    return "BNROM";
}