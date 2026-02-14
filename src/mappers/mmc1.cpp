#include "mmc1.hpp"
#include "../nes_cpu.hpp"
#include "../nes_ppu.hpp"

MMC1::MMC1() {
    reset();
}

void MMC1::reset() {
    ShiftReg = 0;
    ShiftCount = 0;
    Ctrl = 0x0C;
    ChrBank0 = 0;
    ChrBank1 = 0;
    PrgBank = 0;
    PrgMode = (Ctrl >> 2) & 3;
    ChrMode = (Ctrl >> 4) & 1;
    updateBanks();
}

uint8_t MMC1::cpuRead(uint16_t addr) {
    if (addr < 0x8000) {
        if (addr >= 0x6000) {
            return cpu.PrgRAM[addr - 0x6000];
        }
    } else {
        int Slot = (addr < 0xC000) ? 0 : 1;
        size_t base = globalROM.mapper->PRGBankOffset[Slot];
        size_t index = base + (addr & 0x3FFF);
        if (index >= globalROM.PRGRomSize) index %= globalROM.PRGRomSize;
        return globalROM.ROM[index];
    }
    return 0xff;
}

uint8_t MMC1::ppuRead(uint16_t addr) {
    addr &= 0x1FFF; 
    
    if (addr < 0x1000) {
        size_t offset = CHRBankOffset[0] + addr;
        return ppu.ChrData[offset]; 
    } else {
        size_t offset = CHRBankOffset[1] + (addr - 0x1000);
        return ppu.ChrData[offset];
    }
}

void MMC1::cpuWrite(uint16_t addr, uint8_t value) {
    if (addr >= 0x6000 && addr < 0x8000) {
        cpu.PrgRAM[addr - 0x6000] = value;
        return;
    }
    if (addr >= 0x8000) {
        if (value & 0x80) {
            ShiftReg = 0;
            ShiftCount = 0;
            Ctrl |= 0x0C;
            PrgMode = (Ctrl >> 2) & 3;
            updateBanks();
            return;
        }
        ShiftReg >>= 1;
        ShiftReg |= (value & 1) << 4;
        ShiftCount++;
        if (ShiftCount < 5) return;
        uint8_t data = ShiftReg & 0x1F;
        modifyRegister(addr, data);
        ShiftReg = 0;
        ShiftCount = 0;
        PrgMode = (Ctrl >> 2) & 3;
        ChrMode = (Ctrl >> 4) & 1;
        updateBanks();
    }
}

const char *MMC1::getName(void) {
    return "MMC1";
}

void MMC1::modifyRegister(uint16_t addr, uint8_t data) {
    if (addr <= 0x9FFF) {
        Ctrl = data;
    } else if (addr <= 0xBFFF) {
        ChrBank0 = data;
    } else if (addr <= 0xDFFF) {
        ChrBank1 = data;
    } else {
        PrgBank = data;
    }
}

void MMC1::updateBanks() {
    size_t prgSize = globalROM.PRGRomSize;

    if (PrgMode == 0 || PrgMode == 1) {
        setPRGSlot2(0, PrgBank & 0x0E);
    } else if (PrgMode == 2) {
        setPRGSlot(0, 0);
        setPRGSlot(1, PrgBank & 0x0F);
    } else {
        setPRGSlot(0, PrgBank & 0x0F);
        size_t last = (prgSize == 0) ? 0 : (prgSize - 0x4000);
        PRGBankOffset[1] = last;
    }

    if (globalROM.CHRRomSize == 0) {
        CHRBankOffset[0] = 0;
        CHRBankOffset[1] = 0x1000;
    } else {
        if (ChrMode == 0) {
            size_t bank = (ChrBank0 & 0x1E);

            setCHRSlot(0, bank);
            setCHRSlot(1, bank + 1);
        } else {
            size_t b0 = (ChrBank0 & 0x1F);
            size_t b1 = (ChrBank1 & 0x1F);
            setCHRSlot(0, b0);
            setCHRSlot(1, b1);
        }
    }

    switch (Ctrl & 3) {
        case 0:
            ppu.nametableSelect = 0; 
            break;
        case 1:
            ppu.nametableSelect = 1;
            break;
        case 2:
            ppu.nametableSelect = 0;
            break;
        case 3:
            ppu.nametableSelect = 2;
            break;
    }
}
