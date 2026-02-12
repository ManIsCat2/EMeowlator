#include "uxrom.hpp"
#include "../nes_cpu.hpp"
#include "../nes_ppu.hpp"

UxROM::UxROM() {
    reset();
}

void UxROM::reset() {
    PrgBank = 0;
    updateBanks();
}

uint8_t UxROM::cpuRead(uint16_t addr) {
    return cpu.PrgRAM[addr - 0x6000];
}

uint8_t UxROM::cpuReadAfter0x8000(uint16_t addr) {
    if (addr < 0xC000) {
        return globalROM.ROM[PRGBankOffset[0] + (addr - 0x8000)];
    } else {
        return globalROM.ROM[PRGBankOffset[1] + (addr - 0xC000)];
    }
}

void UxROM::cpuWrite(uint16_t addr, uint8_t value) {
    if (addr >= 0x8000) {
        PrgBank = value & 0x0f;
        updateBanks();
    }
}

const char* UxROM::getName(void) {
    return "UxROM";
}

uint8_t UxROM::ppuRead(uint16_t addr) {
    addr &= 0x1FFF;
    return ppu.ChrData[addr];
}

void UxROM::updateBanks() {
    size_t bankCount = globalROM.PRGRomSize / 0x4000;

    size_t bank = PrgBank % bankCount;
    PRGBankOffset[0] = bank * 0x4000;
    PRGBankOffset[1] = (bankCount - 1) * 0x4000;
}
