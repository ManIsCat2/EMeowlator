#include "uxrom.hpp"
#include "../nes_cpu.hpp"
#include "../nes_ppu.hpp"

UxROM::UxROM() {

}

void UxROM::reset() {
    setPRGPages(0, 0);
	setPRGPages(1, -1);
    setCHRPages(0, 0);
}

void UxROM::cpuWrite(uint16_t addr, uint8_t value) {
    if (addr >= 0x8000) {
        setPRGPages(0, value);
        return;
    }
    MapperBase::cpuWrite(addr, value);
}

const char* UxROM::getName(void) {
    return "UxROM";
}