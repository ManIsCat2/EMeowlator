#include "nrom.hpp"
#include "../nes_cpu.hpp"
#include "../nes_ppu.hpp"

NROM::NROM() {
    reset();
}

void NROM::reset() {
    setPRGPage(0, 0);
	setPRGPage(1, 1);
    setCHRPage(0, 0);
}

const char *NROM::getName(void) {
    return "NROM";
}