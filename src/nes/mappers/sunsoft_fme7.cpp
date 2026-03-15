#include "sunsoft_fme7.hpp"
#include "../nes_cpu.hpp"
#include "../nes_ppu.hpp"

SunSoftFME7::SunSoftFME7() {

}

void SunSoftFME7::reset() {
    command = 0;
    workRamValue = 0;

    irqEnabled = false;
    irqCounterEnabled = false;
    irqCounter = 0;;

    setPRGPage(3, -1);
    updateWRAM();
}

void SunSoftFME7::cpuWrite(uint16_t addr, uint8_t value) {
    switch(addr & 0xE000) {
        case 0x8000:
            command = value & 0x0F;
            break;
        case 0xA000:
            switch(command) {
                case 0: case 1: case 2: case 3:
                case 4: case 5: case 6: case 7:
                    setCHRPages(command, value);
                    break;

                case 8:
                    workRamValue = value;
                    updateWRAM();
                    break;

                case 9: case 0xA: case 0xB:
                    setPRGPage(command - 9, value & 0x3F);
                    break;

                case 0xC:
                    switch(value & 3) {
                        case 0: ppu.Mirroring = MirrorMode::VERTICAL; break;
                        case 1: ppu.Mirroring = MirrorMode::HORIZONTAL; break;
                        case 2: ppu.Mirroring = MirrorMode::SCREEN_A; break;
                        case 3: ppu.Mirroring = MirrorMode::SCREEN_B; break;
                    }
                    break;

                case 0xD:
                    irqEnabled = value & 1;
                    irqCounterEnabled = value & 0x80;
                    cpu.IRQPending = false;
                    break;
                case 0xE:
                    irqCounter = (irqCounter & 0xFF00) | value;
                    break;
                case 0xF:
                    irqCounter = (irqCounter & 0x00FF) | (value << 8);
                    break;
            }
            break;
    }
}

const char* SunSoftFME7::getName(void) {
    return "SunSoft FME-7";
}

void SunSoftFME7::clockCPU(void) {
    if(!irqCounterEnabled)
        return;

    irqCounter--;

    if (irqCounter == 0xFFFF) {
        if (irqEnabled) cpu.IRQPending = true;
    }
}

void SunSoftFME7::updateWRAM(void) {
    if (workRamValue & 0x40) {
        if (workRamValue & 0x80) {
		    mapCPUMemory(0x6000, 0x7FFF, globalROM.hasBattery ? SRAM : PRGRam, 0, true, 0x60);
        } else {
            unmapCPUMemory(0x6000, 0x7FFF, 0x60);
        }
	} else {
		mapCPUMemory(0x6000, 0x7FFF, globalROM.ROM, 0, true, 0x60);
	}
}