#include "nrom.hpp"
#include "../nes_cpu.hpp"
#include "../nes_ppu.hpp"

NROM::NROM() {

}

void NROM::reset() {
    setPRGBank(0, 0);
	setPRGBank(1, 1);
    setCHRBank(0, 0);
}

const char *NROM::getName(void) {
    return "NROM";
}