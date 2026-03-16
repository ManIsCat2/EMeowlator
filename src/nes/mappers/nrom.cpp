#include "nrom.hpp"
#include "../nes_cpu.hpp"
#include "../nes_ppu.hpp"

NROM::NROM() {

}

void NROM::reset() {
    setPRGPages(0, 0);
	setPRGPages(1, 1);
    setCHRPages(0, 0);
}

const char *NROM::getName(void) {
    return "NROM";
}