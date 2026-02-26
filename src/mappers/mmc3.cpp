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

    mapCPUMemory(0x6000, 0x7fff, cpu.PrgRAM, 0, true, 0x60, false);

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
            uint8_t bankReg = BankSelect & 7;
            if (bankReg <= 1) {
                value &= 0xFE;
            }
            BankRegisters[bankReg] = value;
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
        setPRGPage(0, BankRegisters[6]);
		setPRGPage(1, BankRegisters[7]);
        setPRGPage(2, -2);
		setPRGPage(3, -1);
    } else {
        setPRGPage(0, -2);
        setPRGPage(1, BankRegisters[7]);
		setPRGPage(2, BankRegisters[6]);
        setPRGPage(3, -1);
    }
}

void MMC3::updateCHR() {
    if (ChrMode == 0) {
        setCHRPage(0, BankRegisters[0] & 0xFE);
		setCHRPage(1, BankRegisters[0] | 0x01);
		setCHRPage(2, BankRegisters[1] & 0xFE);
		setCHRPage(3, BankRegisters[1] | 0x01);
		setCHRPage(4, BankRegisters[2]);
		setCHRPage(5, BankRegisters[3]);
		setCHRPage(6, BankRegisters[4]);
		setCHRPage(7, BankRegisters[5]);
    } else if (ChrMode == 1) {
        setCHRPage(0, BankRegisters[2]);
		setCHRPage(1, BankRegisters[3]);
		setCHRPage(2, BankRegisters[4]);
		setCHRPage(3, BankRegisters[5]);
		setCHRPage(4, BankRegisters[0] & 0xFE);
		setCHRPage(5, BankRegisters[0] | 0x01);
		setCHRPage(6, BankRegisters[1] & 0xFE);
		setCHRPage(7, BankRegisters[1] | 0x01);
    }
}
