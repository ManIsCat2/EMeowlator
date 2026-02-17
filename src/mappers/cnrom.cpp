#include "cnrom.hpp"
#include "../nes_cpu.hpp"
#include "../nes_ppu.hpp"

CNROM::CNROM() {
    reset();
}

void CNROM::reset() {
    setPRGSlot(0, 0);
    setCHRSlot(0, 0);
}

void CNROM::cpuWrite(uint16_t addr, uint8_t value) {
    if (addr >= 0x8000) {
        setCHRSlot(0, value);
        return;
    }
    MapperBase::cpuWrite(addr, value);
}

const char* CNROM::getName(void) {
    return "CNROM";
}