#include "mmc3.hpp"
#include "../nes_cpu.hpp"
#include "../nes_ppu.hpp"

MMC3::MMC3() {
    reset();
}

void MMC3::reset() {
    BankSelect = 0;
    PrgMode = 0;
    ChrMode = 0;

    for (int i = 0; i < 8; i++) BankRegisters[i] = 0;

    IRQReload = 0;
    IRQCounter = 0;
    IRQEnabled = false;
    LastA12 = false;

    updatePRG();
    updateCHR();
}

uint8_t MMC3::cpuRead(uint16_t addr) {
    if (addr < 0x8000) {
        if (addr >= 0x6000) {
            return cpu.PrgRAM[addr - 0x6000];
        }
    } else {
        if (addr < 0xA000) return globalROM.ROM[PRGBankOffset[0] + (addr - 0x8000)];
        if (addr < 0xC000) return globalROM.ROM[PRGBankOffset[1] + (addr - 0xA000)];
        if (addr < 0xE000) return globalROM.ROM[PRGBankOffset[2] + (addr - 0xC000)];
        return globalROM.ROM[PRGBankOffset[3] + (addr - 0xE000)];
    }
    return 0xff;
}

void MMC3::cpuWrite(uint16_t addr, uint8_t value) {
    if (addr >= 0x6000 && addr < 0x8000) {
        cpu.PrgRAM[addr - 0x6000] = value;
    } else if (addr >= 0x8000 && addr <= 0x9FFF) {
        if ((addr & 1) == 0) {
            BankSelect = value;
            PrgMode = (value >> 6) & 1;
            ChrMode = (value >> 7) & 1;
        } else {
            BankRegisters[BankSelect & 7] = value;
            updatePRG();
            updateCHR();
        }
    } else if (addr >= 0xA000 && addr <= 0xBFFF) {
        if ((addr & 1) == 0) {
            ppu.Mirroring = (value & 1) ? MirrorMode::HORIZONTAL : MirrorMode::VERTICAL;
        }
    } else if (addr >= 0xC000 && addr <= 0xDFFF) {
        if ((addr & 1) == 0) {
            IRQReload = value;
        } else {
            IRQCounter = 0;
        }
    } else if (addr >= 0xE000) {
        if ((addr & 1) == 0) {
            IRQEnabled = false;
            cpu.doIRQ = false;
        } else {
            IRQEnabled = true;
        }
    }
}

const char* MMC3::getName(void) {
    return "MMC3";
}

uint8_t MMC3::ppuRead(uint16_t addr) {
    addr &= 0x1FFF;

    bool a12 = addr & 0x1000;

    clockIRQ(a12);

    size_t bank = addr / 0x400;
    size_t offset = addr % 0x400;

    return ppu.ChrData[CHRBankOffset[bank] + offset];
}

void MMC3::clockIRQ(bool a12) {
    if (!LastA12 && a12) {
        if (IRQCounter == 0) {
            IRQCounter = IRQReload;
        } else {
            IRQCounter--;
        }

        if (IRQCounter == 0 && IRQEnabled) {
            cpu.doIRQ = true;
        }
    }

    LastA12 = a12;
}

void MMC3::updatePRG() {
    size_t bankCount = globalROM.PRGRomSize / 0x2000;

    size_t lastBank = bankCount - 1;
    size_t secondLast = bankCount - 2;

    if (PrgMode == 0) {
        PRGBankOffset[0] = (BankRegisters[6] % bankCount) * 0x2000;
        PRGBankOffset[1] = (BankRegisters[7] % bankCount) * 0x2000;
        PRGBankOffset[2] = secondLast * 0x2000;
        PRGBankOffset[3] = lastBank * 0x2000;
    } else {
        PRGBankOffset[0] = secondLast * 0x2000;
        PRGBankOffset[1] = (BankRegisters[7] % bankCount) * 0x2000;
        PRGBankOffset[2] = (BankRegisters[6] % bankCount) * 0x2000;
        PRGBankOffset[3] = lastBank * 0x2000;
    }
}

void MMC3::updateCHR() {
    if (globalROM.CHRRomSize == 0) {
        for (int i = 0; i < 8; i++) {
            CHRBankOffset[i] = (BankRegisters[i] % 8) * 0x400;
        }
        return;
    }
    size_t chrCount = globalROM.CHRRomSize / 0x400;

    if (ChrMode == 0) {
        CHRBankOffset[0] = (BankRegisters[0] & ~1) % chrCount * 0x400;
        CHRBankOffset[1] = (BankRegisters[0] | 1) % chrCount * 0x400;
        CHRBankOffset[2] = (BankRegisters[1] & ~1) % chrCount * 0x400;
        CHRBankOffset[3] = (BankRegisters[1] | 1) % chrCount * 0x400;
        CHRBankOffset[4] = (BankRegisters[2] % chrCount) * 0x400;
        CHRBankOffset[5] = (BankRegisters[3] % chrCount) * 0x400;
        CHRBankOffset[6] = (BankRegisters[4] % chrCount) * 0x400;
        CHRBankOffset[7] = (BankRegisters[5] % chrCount) * 0x400;
    } else {
        CHRBankOffset[4] = (BankRegisters[0] & ~1) % chrCount * 0x400;
        CHRBankOffset[5] = (BankRegisters[0] | 1) % chrCount * 0x400;
        CHRBankOffset[6] = (BankRegisters[1] & ~1) % chrCount * 0x400;
        CHRBankOffset[7] = (BankRegisters[1] | 1) % chrCount * 0x400;
        CHRBankOffset[0] = (BankRegisters[2] % chrCount) * 0x400;
        CHRBankOffset[1] = (BankRegisters[3] % chrCount) * 0x400;
        CHRBankOffset[2] = (BankRegisters[4] % chrCount) * 0x400;
        CHRBankOffset[3] = (BankRegisters[5] % chrCount) * 0x400;
    }
}
