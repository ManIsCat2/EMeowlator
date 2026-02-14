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

uint8_t UxROM::cpuRead(uint16_t addr) {
    if (addr < 0x8000) {
        if (addr >= 0x6000) {
            return cpu.PrgRAM[addr - 0x6000];
        }
    } else {
        if (addr < 0xC000) {
            return globalROM.ROM[PRGBankOffset[0] + (addr - 0x8000)];
        } else {
            return globalROM.ROM[PRGBankOffset[1] + (addr - 0xC000)];
        }
    }
    return 0xff;
}

void UxROM::cpuWrite(uint16_t addr, uint8_t value) {
    if (addr >= 0x8000) {
        setPRGSlot(0, value);
    }
}

const char* UxROM::getName(void) {
    return "UxROM";
}

uint8_t UxROM::ppuRead(uint16_t addr) {
    addr &= 0x1FFF;
    return ppu.ChrData[addr];
}