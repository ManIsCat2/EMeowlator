#include "bnrom.hpp"
#include "../nes_cpu.hpp"
#include "../nes_ppu.hpp"

BNROM::BNROM() {

}

void BNROM::reset() {
    setPRGPages(0, 0);
    setCHRPages(0, 0);
}

void BNROM::cpuWrite(uint16_t addr, uint8_t value) {
    if (addr >= 0x8000) {
        setPRGPages(0, value);
        return;
    }
    MapperBase::cpuWrite(addr, value);
}

const char* BNROM::getName(void) {
    return "BNROM";
}