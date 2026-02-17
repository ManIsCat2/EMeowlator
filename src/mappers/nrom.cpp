#include "nrom.hpp"
#include "../nes_cpu.hpp"
#include "../nes_ppu.hpp"

NROM::NROM() {
    reset();
}

void NROM::reset() {
    setPRGSlot(0, 0);
	setPRGSlot(1, 1);
    setCHRSlot(0, 0);
}

const char* NROM::getName(void) {
    return "NROM";
}