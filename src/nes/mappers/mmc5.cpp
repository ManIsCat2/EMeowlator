#include "mmc5.hpp"
#include "../nes_cpu.hpp"
#include "../nes_ppu.hpp"
#include "../nes_rom.hpp"

#include <cstring>

MMC5::MMC5() {

}

void MMC5::reset() {
    prgMode = 3;
    chrMode = 3;

    memset(prgRegs, 0, sizeof(prgRegs));
    memset(chrRegs, 0, sizeof(chrRegs));

    prgRegs[4] = 0xFF;

    nametableMap[0] = 0;
    nametableMap[1] = 0;
    nametableMap[2] = 0;
    nametableMap[3] = 0;

    fillTile = 0;
    fillAttr = 0;

    irqEnabled = false;
    irqScanline = 0;
    irqPending = false;
    currentScanline = 0;

    mulA = 0;
    mulB = 0;

    chrUpperBits = 0;

    EXRAMMode = 0;

    memset(EXRAM, 0, sizeof(EXRAM));

    uint8_t *mem = globalROM.hasBattery ? SRAM : PRGRam;

    //force wram and dont use any sram (for now)
    mapCPUMemory(0x6000, 0x7FFF, PRGRam, 0, true);
    updateState();
}

const char* MMC5::getName() {
    return "MMC5";
}

uint8_t MMC5::cpuRead(uint16_t addr) {
    if (addr >= 0x5C00 && addr <= 0x5FFF) {
        switch (EXRAMMode) {
            case 0:
            case 1:
            case 2:
                return EXRAM[addr & 0x3FF];

            case 3:
                return cpu.dataBus;
        }
    }

    switch (addr) {
        case 0x5204: {
            uint8_t val = 0;

            if (irqPending) val |= 0x80;
            if (ppu.ScanLine < 240) val |= 0x40;

            irqPending = false;
            cpu.IRQPending = false;

            return val;
        }

        case 0x5205:
            return (mulA * mulB) & 0xFF;

        case 0x5206:
            return ((mulA * mulB) >> 8) & 0xFF;
    }

    return MapperBase::cpuRead(addr);
}

void MMC5::cpuWrite(uint16_t addr, uint8_t value) {
    if (addr >= 0x5C00 && addr <= 0x5FFF) {
        switch (EXRAMMode) {
            case 0:
            case 1:
                EXRAM[addr & 0x3FF] = value;
                break;

            case 2:
                EXRAM[addr & 0x3FF] = value;
                break;

            case 3:
                break;
        }

        return;
    }

    switch (addr) {
        case 0x5100:
            prgMode = value & 0x03;
            break;

        case 0x5101:
            chrMode = value & 0x03;
            break;

        case 0x5104:
            EXRAMMode = value & 0x03;
            break;

        case 0x5105:
            nametableMap[0] = (value >> 0) & 0x03;
            nametableMap[1] = (value >> 2) & 0x03;
            nametableMap[2] = (value >> 4) & 0x03;
            nametableMap[3] = (value >> 6) & 0x03;
            break;

        case 0x5106:
            fillTile = value;
            break;

        case 0x5107:
            fillAttr = value & 0x03;
            break;

        case 0x5113:
            break;

        case 0x5114:
        case 0x5115:
        case 0x5116:
        case 0x5117:
            prgRegs[addr - 0x5113] = value;
            break;

        case 0x5120:
        case 0x5121:
        case 0x5122:
        case 0x5123:
        case 0x5124:
        case 0x5125:
        case 0x5126:
        case 0x5127:
            chrRegs[addr - 0x5120] = (chrUpperBits << 8) | value;
            updateCHR(true);
            break;

        case 0x5128:
        case 0x5129:
        case 0x512A:
        case 0x512B:
            chrRegs[8 + (addr - 0x5128)] = (chrUpperBits << 8) | value;
            updateCHR(false);
            break;

        case 0x5130:
            chrUpperBits = value & 0x03;
            break;

        case 0x5203:
            irqScanline = value;
            break;

        case 0x5204:
            irqEnabled = (value & 0x80) != 0;

            if (!irqEnabled) {
                cpu.IRQPending = false;
            }
            break;

        case 0x5205:
            mulA = value;
            break;

        case 0x5206:
            mulB = value;
            break;
        default:
            MapperBase::cpuWrite(addr, value);
            break;
    }

    updateState();
}

void MMC5::updatePRG() {
    switch (prgMode) {
        case 0:
            setPRGPages(0, (prgRegs[4] & 0x7C), BANK_32K);
            break;

        case 1:
            setPRGPages(0, (prgRegs[2] & 0x7E), BANK_16K);
            setPRGPages(2, (prgRegs[4] & 0x7E), BANK_16K);
            break;

        case 2:
            setPRGPages(0, prgRegs[1], BANK_8K);
            setPRGPages(1, prgRegs[2], BANK_8K);
            setPRGPages(2, prgRegs[3], BANK_8K);
            setPRGPages(3, prgRegs[4], BANK_8K);
            break;

        case 3:
            setPRGPages(0, prgRegs[1]);
            setPRGPages(1, prgRegs[2]);
            setPRGPages(2, prgRegs[3]);
            setPRGPages(3, prgRegs[4]);
            break;
    }
}

void MMC5::updateCHR(bool sprite) {
    if (sprite) {
        switch (chrMode) {
            case 0: {
                uint16_t bank = chrRegs[7] << 3;

                for (int i = 0; i < 8; i++) {
                    setCHRPages(i, bank + i, BANK_1K);
                }
                break;
            }

            case 1: {
                uint16_t bank0 = chrRegs[3] << 2;
                uint16_t bank1 = chrRegs[7] << 2;

                for (int i = 0; i < 4; i++) {
                    setCHRPages(i, bank0 + i, BANK_1K);
                }

                for (int i = 0; i < 4; i++) {
                    setCHRPages(4 + i, bank1 + i, BANK_1K);
                }

                break;
            }

            case 2: {
                for (int i = 0; i < 4; i++) {
                    uint16_t bank = chrRegs[i * 2 + 1] << 1;

                    setCHRPages(i * 2 + 0, bank + 0, BANK_1K);
                    setCHRPages(i * 2 + 1, bank + 1, BANK_1K);
                }

                break;
            }

            case 3:
                for (int i = 0; i < 8; i++) {
                    setCHRPages(i, chrRegs[i], BANK_1K);
                }

                break;
        }

    } else {
        switch (chrMode) {
            case 0: {
                uint16_t bank = chrRegs[11] << 3;
                for (int i = 0; i < 8; i++) {
                    setCHRPages(i, bank + i, BANK_1K);
                }

                break;
            }

            case 1: {
                uint16_t bank0 = chrRegs[9] << 2;
                uint16_t bank1 = chrRegs[11] << 2;

                for (int i = 0; i < 4; i++) {
                    setCHRPages(i, bank0 + i, BANK_1K);
                }

                for (int i = 0; i < 4; i++) {
                    setCHRPages(4 + i, bank1 + i, BANK_1K);
                }

                break;
            }

            case 2: {
                for (int i = 0; i < 4; i++) {
                    uint16_t bank = chrRegs[8 + i] << 1;

                    setCHRPages(i * 2 + 0, bank + 0, BANK_1K);
                    setCHRPages(i * 2 + 1, bank + 1, BANK_1K);
                }

                break;
            }

            case 3:
                for (int i = 0; i < 4; i++) {
                    uint16_t bank = chrRegs[8 + i];

                    setCHRPages(i + 0, bank, BANK_1K);
                    setCHRPages(i + 4, bank, BANK_1K);
                }

            break;
        }
    }
}

void MMC5::updateState() {
    updatePRG();
}

uint8_t MMC5::fillModeRead(uint16_t addr) {
    if ((addr & 0x03C0) == 0x03C0) {
        uint8_t attr = fillAttr & 0x03;
        return attr | (attr << 2) | (attr << 4) | (attr << 6);
    }

    return fillTile;
}

uint8_t MMC5::readVRAM(uint16_t addr) {
    addr &= 0x0FFF;

    uint8_t nt = (addr >> 10) & 0x03;
    uint16_t offs = addr & 0x03FF;

    switch (nametableMap[nt]) {
        case 0:
            return ppu.VRAM[offs];

        case 1:
            return ppu.VRAM[0x400 + offs];

        case 2:
            return EXRAM[offs];

        case 3:
            return fillModeRead(offs);
    }

    return 0;
}

void MMC5::writeVRAM(uint16_t addr, uint8_t value) {
    addr &= 0x0FFF;

    uint8_t nt = (addr >> 10) & 0x03;
    uint16_t offs = addr & 0x03FF;

    switch (nametableMap[nt]) {
        case 0:
            ppu.VRAM[offs] = value;
            break;

        case 1:
            ppu.VRAM[0x400 + offs] = value;
            break;

        case 2:
            if (EXRAMMode != 3) {
                EXRAM[offs] = value;
            }

            break;
    }
}

void MMC5::clockCPU(void) {

}

void MMC5::clockPPU(void) {
    if (ppu.Dot == 257) {
        currentScanline++;

        if (currentScanline == irqScanline) {
            irqPending = true;

            if (irqEnabled) {
                cpu.IRQPending = true;
            }
        }

        if (currentScanline >= 262) {
            currentScanline = 0;
            irqPending = false;
        }
    }
}

uint8_t MMC5::readCHR(uint16_t addr, bool sprite) {
    addr &= 0x1FFF;

    if (usingExtendedAttributes() && !sprite) {
        uint8_t ex = getEXRAMByte(ppu.VRAMAddr);
        uint32_t bank = ex & 0x3F;
        uint32_t finalAddr = (bank * 0x400) | (addr & 0x3FF);

        return ppu.ChrData[finalAddr & (globalROM.CHRRomSize - 1)];
    }

    return MapperBase::readCHR(addr, sprite);
}

bool MMC5::usingExtendedAttributes() {
    if (EXRAMMode != 1) {
        return false;
    }

    uint8_t nt = (ppu.VRAMAddr >> 10) & 0x03;

    return nametableMap[nt] == 2;
}

uint8_t MMC5::getEXRAMByte(uint16_t vramAddr) {

    uint16_t tileIndex = ((vramAddr >> 5) & 0x1F) * 32 + (vramAddr & 0x1F);

    return EXRAM[tileIndex];
}