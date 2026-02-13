#include "mmc2.hpp"
#include "../nes_cpu.hpp"
#include "../nes_ppu.hpp"

MMC2::MMC2() {
    reset();
}

void MMC2::reset() {
    PrgBank = 0;

    ChrBankFD[0] = 0;
    ChrBankFD[1] = 0;
    ChrBankFE[0] = 0;
    ChrBankFE[1] = 0;

    Latch[0] = 0;
    Latch[1] = 0;

    updatePRG();
    updateCHR();
}

uint8_t MMC2::cpuRead(uint16_t addr) {
    if (addr < 0x8000) {
        if (addr >= 0x6000) {
            return cpu.PrgRAM[addr - 0x6000];
        }
    } else {
        if (addr < 0xA000) {
            return globalROM.ROM[PRGBankOffset[0] + (addr - 0x8000)];
        } else {
            return globalROM.ROM[PRGBankOffset[1] + (addr - 0xA000)];
        }
    }
    return 0xff;
}

void MMC2::cpuWrite(uint16_t addr, uint8_t value) {
    if (addr >= 0xA000 && addr <= 0xAFFF) {
        PrgBank = value;
        updatePRG();
    } else if (addr >= 0xB000 && addr <= 0xBFFF) {
        ChrBankFD[0] = value;
        updateCHR();
    } else if (addr >= 0xC000 && addr <= 0xCFFF) {
        ChrBankFE[0] = value;
        updateCHR();
    } else if (addr >= 0xD000 && addr <= 0xDFFF) {
        ChrBankFD[1] = value;
        updateCHR();
    } else if (addr >= 0xE000 && addr <= 0xEFFF) {
        ChrBankFE[1] = value;
        updateCHR();
    }
}

const char* MMC2::getName(void) {
    return "MMC2";
}

uint8_t MMC2::ppuRead(uint16_t addr) {
    addr &= 0x1FFF;

    if (addr == 0x0FD8) {
        Latch[0] = 0;
        updateCHR();
    } else if (addr == 0x0FE8) {
        Latch[0] = 1;
        updateCHR();
    } else if (addr == 0x1FD8) {
        Latch[1] = 0;
        updateCHR();
    } else if (addr == 0x1FE8) {
        Latch[1] = 1;
        updateCHR();
    }

    if (addr < 0x1000) {
        return ppu.ChrData[CHRBankOffset[0] + addr];
    } else {
        return ppu.ChrData[CHRBankOffset[1] + (addr - 0x1000)];
    }
}

void MMC2::updatePRG() {
    size_t bankCount = globalROM.PRGRomSize / 0x2000;
    size_t bank = PrgBank % bankCount;

    PRGBankOffset[0] = bank * 0x2000;
    PRGBankOffset[1] = (bankCount - 3) * 0x2000;
}

void MMC2::updateCHR() {
    uint8_t bank0 = (Latch[0] == 0) ? ChrBankFD[0] : ChrBankFE[0];
    uint8_t bank1 = (Latch[1] == 0) ? ChrBankFD[1] : ChrBankFE[1];

    size_t chrCount = globalROM.CHRRomSize / 0x1000;

    bank0 %= chrCount;
    bank1 %= chrCount;
    CHRBankOffset[0] = bank0 * 0x1000;
    CHRBankOffset[1] = bank1 * 0x1000;
}