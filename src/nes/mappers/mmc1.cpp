#include "mmc1.hpp"
#include "../nes_cpu.hpp"
#include "../nes_ppu.hpp"

MMC1::MMC1() {

}

void MMC1::reset() {
    shiftCount  = 0;
    WriteBuffer = 0;

    control = 0x0C;
    chrReg0 = 0;
    chrReg1 = 0;
    prgReg = 0;

    modifyRegister(0x8000, control);
    mapCPUMemory(0x6000, 0x7FFF, getNESRom()->hasBattery ? SRAM : PRGRam, 0, true);
    updateBanks();
}

void MMC1::cpuWrite(uint16_t addr, uint8_t value) {
    if (addr < 0x8000) {
        MapperBase::cpuWrite(addr, value);
        return;
    }

    if (value & 0x80) {
        shiftCount = 0;
        WriteBuffer = 0;

        control |= 0x0C;
        modifyRegister(0x8000, control);
        updateBanks();
        return;
    }

    WriteBuffer >>= 1;
    WriteBuffer |= (value & 1) << 4;

    shiftCount++;

    if (shiftCount == 5) {
        modifyRegister(addr, WriteBuffer);
        updateBanks();

        shiftCount = 0;
        WriteBuffer = 0;
    }
}

const char *MMC1::getName(void) {
    return "MMC1";
}

void MMC1::modifyRegister(uint16_t addr, uint8_t val) {
    switch (addr & 0xE000) {
        case 0x8000:
            control = val;
            switch (val & 0x03) {
                case 0: ppu.Mirroring = MirrorMode::SCREEN_A; break;
                case 1: ppu.Mirroring = MirrorMode::SCREEN_B; break;
                case 2: ppu.Mirroring = MirrorMode::VERTICAL; break;
                case 3: ppu.Mirroring = MirrorMode::HORIZONTAL; break;
            }

            slotSelect = (val & 0x04) != 0;
            PrgMode = (val & 0x08) != 0;
            ChrMode = (val & 0x10) != 0;
            break;

        case 0xA000:
            chrReg0 = val & 0x1F;
            break;

        case 0xC000:
            chrReg1 = val & 0x1F;
            break;

        case 0xE000:
            prgReg = val & 0x0F;
            break;
    }
}

void MMC1::updateBanks() {
    uint8_t prgBankSelect = 0;

    if (getNESRom()->PRGRomSize == 0x80000) {
        prgBankSelect = (chrReg0 & 0x10);
    }

    if (PrgMode) {
        if (slotSelect) {
            setPRGBank(0, prgReg | prgBankSelect);
            setPRGBank(1, 0x0F | prgBankSelect);
        } else {
            setPRGBank(0, 0x00 | prgBankSelect);
            setPRGBank(1, prgReg | prgBankSelect);
        }
    } else {
        setPRGBank(0, (prgReg & 0xFE) | prgBankSelect, BANK_2K);
    }

    if (ChrMode) {
        setCHRBank(0, chrReg0);
        setCHRBank(1, chrReg1);
    } else {
        setCHRBank(0, chrReg0 & 0x1E);
        setCHRBank(1, (chrReg0 & 0x1E) + 1);
    }
}