#include "uxrom.hpp"
#include "../nes_cpu.hpp"
#include "../nes_ppu.hpp"

UxROM::UxROM() {
    reset();
}

void UxROM::reset() {
    setPRGSlot(0, 0);
	setPRGSlot(1, -1);
}

void UxROM::cpuWrite(uint16_t addr, uint8_t value) {
    if (addr >= 0x8000) {
        setPRGSlot(0, value);
        return;
    }
    MapperBase::cpuWrite(addr, value);
}

const char* UxROM::getName(void) {
    return "UxROM";
}

uint8_t UxROM::ppuRead(uint16_t addr) {
    addr &= 0x1FFF;
    return ppu.ChrData[addr];
}