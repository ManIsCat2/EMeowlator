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

    mapCPUMemory(0x6000, 0x7fff, cpu.PrgRAM, 0, true, 0x60);

    updatePRG();
    updateCHR();
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
    bool a12 = (addr & 0x1fff) & 0x1000;
    clockIRQ(a12);

    return MapperBase::ppuRead(addr);
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
    if (PrgMode == 0) {
        setPRGSlot(0, BankRegisters[6]);
		setPRGSlot(1, BankRegisters[7]);
        setPRGSlot(2, -2);
		setPRGSlot(3, -1);
    } else {
        setPRGSlot(0, -2);
        setPRGSlot(1, BankRegisters[7]);
		setPRGSlot(2, BankRegisters[6]);
        setPRGSlot(3, -1);
    }
}

void MMC3::updateCHR() {
    if (globalROM.CHRRomSize == 0) {
        for (int i = 0; i < 8; i++) {
            CHRBankOffset[i] = (BankRegisters[i] % 8) * 0x400;
        }
        return;
    }

    if (ChrMode == 0) {
        setCHRSlot(0, BankRegisters[0] & 0xFE);
		setCHRSlot(1, BankRegisters[0] | 0x01);
		setCHRSlot(2, BankRegisters[1] & 0xFE);
		setCHRSlot(3, BankRegisters[1] | 0x01);
		setCHRSlot(4, BankRegisters[2]);
		setCHRSlot(5, BankRegisters[3]);
		setCHRSlot(6, BankRegisters[4]);
		setCHRSlot(7, BankRegisters[5]);
    } else {
        setCHRSlot(0, BankRegisters[2]);
		setCHRSlot(1, BankRegisters[3]);
		setCHRSlot(2, BankRegisters[4]);
		setCHRSlot(3, BankRegisters[5]);
		setCHRSlot(4, BankRegisters[0] & 0xFE);
		setCHRSlot(5, BankRegisters[0] | 0x01);
		setCHRSlot(6, BankRegisters[1] & 0xFE);
		setCHRSlot(7, BankRegisters[1] | 0x01);
    }
}
