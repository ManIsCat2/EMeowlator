#include "nina01.hpp"
#include "../nes_cpu.hpp"
#include "../nes_ppu.hpp"

NINA01::NINA01() {
    reset();
}

void NINA01::reset() {
    setPRGSlot(0, 0);
    mapCPUMemory(0x6000, 0x7fff, cpu.PrgRAM, 0, true, 0x6000 >> 8);
}

void NINA01::cpuWrite(uint16_t addr, uint8_t value) {
    switch(addr) {
        case 0x7FFD: setPRGSlot(0, value & 0x01); return;
        case 0x7FFE: setCHRSlot(0, value & 0x0F); return;
        case 0x7FFF: setCHRSlot(1, value & 0x0F); return;
    }
    MapperBase::cpuWrite(addr, value);
}

const char* NINA01::getName(void) {
    return "NINA01";
}