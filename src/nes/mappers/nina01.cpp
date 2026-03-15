#include "nina01.hpp"
#include "../nes_cpu.hpp"
#include "../nes_ppu.hpp"

NINA01::NINA01() {

}

void NINA01::reset() {
    setPRGPage(0, 0);
    mapCPUMemory(0x6000, 0x7fff, PRGRam, 0, true, 0x60);
}

void NINA01::cpuWrite(uint16_t addr, uint8_t value) {
    switch(addr) {
        case 0x7FFD: setPRGPage(0, value & 0x01); return;
        case 0x7FFE: setCHRPage(0, value & 0x0F); return;
        case 0x7FFF: setCHRPage(1, value & 0x0F); return;
    }
    MapperBase::cpuWrite(addr, value);
}

const char* NINA01::getName(void) {
    return "NINA01";
}