#include "uxrom.hpp"
#include "../nes_cpu.hpp"
#include "../nes_ppu.hpp"

UxROM::UxROM() {

}

void UxROM::reset() {
    PRGBank = 0;

    setPRGPages(0, 0);
	setPRGPages(1, -1);
    setCHRPages(0, 0);
}

void UxROM::cpuWrite(uint16_t addr, uint8_t value) {
    if (addr >= 0x8000) {
        PRGBank = value;
        setPRGPages(0, PRGBank);
        return;
    }
    MapperBase::cpuWrite(addr, value);
}

const char* UxROM::getName(void) {
    return "UxROM";
}