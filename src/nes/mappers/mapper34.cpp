#include "mapper34.hpp"
#include "../nes_cpu.hpp"
#include "../nes_ppu.hpp"

Mapper34::Mapper34(bool isNina01) : nina01(isNina01) {

}

void Mapper34::reset() {
    setPRGPages(0, 0);
    if (!nina01) { 
        setCHRPages(0, 0);
    } else {
        mapCPUMemory(0x6000, 0x7fff, PRGRam, 0, true);
    }
}

void Mapper34::cpuWrite(uint16_t addr, uint8_t value) {
    if (!nina01) {
        if (addr >= 0x8000) {
            setPRGPages(0, value);
            return;
        }
    } else {
        switch(addr) {
            case 0x7FFD: setPRGPages(0, value & 0x01); return;
            case 0x7FFE: setCHRPages(0, value & 0x0F); return;
            case 0x7FFF: setCHRPages(1, value & 0x0F); return;
        }
    }
    MapperBase::cpuWrite(addr, value);
}

const char* Mapper34::getName(void) {
    return nina01 ? "Nina001" : "BNROM";
}