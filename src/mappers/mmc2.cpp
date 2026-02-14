#include "mmc2.hpp"
#include "../nes_cpu.hpp"
#include "../nes_ppu.hpp"

MMC2::MMC2() {
    reset();
}

void MMC2::reset() {
    ChrBankFD[0] = 0;
    ChrBankFD[1] = 0;
    ChrBankFE[0] = 0;
    ChrBankFE[1] = 0;

    Latch[0] = 0;
    Latch[1] = 0;

    setPRGSlot(1, -3);
	setPRGSlot(2, -2);
	setPRGSlot(3, -1);
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
        setPRGSlot(0, value & 0x0F);
    } else if (addr >= 0xB000 && addr <= 0xBFFF) {
        ChrBankFD[0] = value & 0x1f;
        setCHRSlot(0, ChrBankFD[Latch[0]]);
    } else if (addr >= 0xC000 && addr <= 0xCFFF) {
        ChrBankFD[1] = value & 0x1f;
        setCHRSlot(0, ChrBankFD[Latch[0]]);
    } else if (addr >= 0xD000 && addr <= 0xDFFF) {
        ChrBankFE[0] = value & 0x1f;
        setCHRSlot(1, ChrBankFE[Latch[1]]);
    } else if (addr >= 0xE000 && addr <= 0xEFFF) {
        ChrBankFE[1] = value & 0x1f;
        setCHRSlot(1, ChrBankFE[Latch[1]]);
    } else if (addr >= 0xF000) {
        ppu.Mirroring = ((value & 0x01) == 0x01) ? MirrorMode::HORIZONTAL : MirrorMode::VERTICAL;
    }
}

const char* MMC2::getName(void) {
    return "MMC2";
}

uint8_t MMC2::ppuRead(uint16_t addr) {
    addr &= 0x1FFF;

    if (ChrUpdate) {
        setCHRSlot(0, ChrBankFD[Latch[0]]);
		setCHRSlot(1, ChrBankFE[Latch[1]]);
		ChrUpdate = false;
    }

    if (addr == 0x0FD8) {
        Latch[0] = 0;
        ChrUpdate = true;
    } else if (addr == 0x0FE8) {
        Latch[0] = 1;
        ChrUpdate = true;
    } else if (addr >= 0x1FD8 && addr <= 0x1FDF) {
        Latch[1] = 0;
        ChrUpdate = true;
    } else if(addr >= 0x1FE8 && addr <= 0x1FEF) {
        Latch[1] = 1;
        ChrUpdate = true;
    }

    if (addr < 0x1000) {
        return ppu.ChrData[CHRBankOffset[0] + addr];
    } else {
        return ppu.ChrData[CHRBankOffset[1] + (addr - 0x1000)];
    }
}