#include "mbc3.hpp"
#include "../gb_rom.hpp"
#include "../gb_cpu.hpp"

MBC3::MBC3() {

}

const char* MBC3::getName(void) {
    return "MBC3";
}

void MBC3::reset() {
    ramEnable = false;
    romBank = 1;
    ramBank = 0;

    mapCPUMemory(0x0000, 0x3FFF, getGBRom()->ROM, 0, false);
    updateBanks();
}

void MBC3::updateBanks() {
    GbROM *rom = getGBRom();

    uint32_t numRomBanks = rom->RomSize / 0x4000;
    uint32_t activeRomBank = (romBank == 0) ? 1 : (romBank & 0x7F);
    mapCPUMemory(0x4000, 0x7FFF, rom->ROM, (activeRomBank % numRomBanks) * 0x4000, false);
}

uint8_t MBC3::cpuRead(uint16_t addr) {
    if (addr >= 0xA000 && addr <= 0xBFFF) {
        if (ramEnable && getGBRom()->hasRAM() && cartRAM && ramBank <= 0x03) {
            return cartRAM[(ramBank * 0x2000) + (addr - 0xA000)];
        }
    }
    
    return MBCBase::cpuRead(addr);
}

void MBC3::cpuWrite(uint16_t addr, uint8_t value) {
    if (addr >= 0x0000 && addr <= 0x1FFF) {
        ramEnable = ((value & 0x0F) == 0x0A);
        return;
    }

    if (addr >= 0x2000 && addr <= 0x3FFF) {
        romBank = value & 0x7F;
        updateBanks();
        return;
    }

    if (addr >= 0x4000 && addr <= 0x5FFF) {
        ramBank = value;
        return;
    }

    if (addr >= 0xA000 && addr <= 0xBFFF) {
        if (ramEnable && getGBRom()->hasRAM() && cartRAM && ramBank <= 0x03) {
            cartRAM[(ramBank * 0x2000) + (addr - 0xA000)] = value;
        }
        return;
    }

    if (addr >= 0x6000 && addr <= 0x7FFF) {
        return;
    }

    MBCBase::cpuWrite(addr, value);
}