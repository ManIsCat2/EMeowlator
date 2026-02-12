#include "nrom.hpp"
#include "../nes_cpu.hpp"
#include "../nes_ppu.hpp"

NROM::NROM() {
    reset();
}

void NROM::reset() {

}

uint8_t NROM::cpuRead(uint16_t addr) {
    return cpu.PrgRAM[addr - 0x6000];
}

uint8_t NROM::cpuReadAfter0x8000(uint16_t addr) {
    return globalROM.ROM[addr - 0x8000];
}

void NROM::cpuWrite(uint16_t addr, uint8_t value) {
    if (addr >= 0x6000 && addr < 0x8000) {
        cpu.PrgRAM[addr - 0x6000] = value;
    }
}

const char* NROM::getName(void) {
    return "NROM";
}

uint8_t NROM::ppuRead(uint16_t addr) {
    addr &= 0x1FFF;
    return ppu.ChrData[addr];
}