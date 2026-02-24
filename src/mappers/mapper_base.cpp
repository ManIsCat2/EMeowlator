#include "mapper_base.hpp"
#include "../nes_rom.hpp"
#include "../main.hpp"

void MapperBase::setCHRSlot(uint16_t slot, uint16_t val, uint32_t offset) {
    uint32_t chrSlotSize = getCHRSlotSize();
    uint32_t bankOffset = ((val * chrSlotSize) + offset) & (globalROM.CHRRomSize - 1);
    uint16_t ppuStart = slot * chrSlotSize;
    uint16_t ppuEnd = ppuStart + chrSlotSize - 1;

    mapPPUMemory(ppuStart, ppuEnd, ppu.ChrData.data(), bankOffset, globalROM.CHRRomSize == 0);
}
void MapperBase::setCHRSlot8(uint16_t slot, uint16_t val, uint32_t offset) {
    setCHRSlot4(slot, val, offset);
    setCHRSlot4(slot*2+1, val+4, offset);
}
void MapperBase::setCHRSlot4(uint16_t slot, uint16_t val, uint32_t offset) {
    setCHRSlot2(slot*2, val, offset);
    setCHRSlot2(slot*2+1, val+2, offset);
}
void MapperBase::setCHRSlot2(uint16_t slot, uint16_t val, uint32_t offset) {
    setCHRSlot(slot*2, val, offset);
    setCHRSlot(slot*2+1, val+1, offset);
}

void MapperBase::setPRGSlot(uint16_t slot, uint16_t val, uint32_t offset) {
    uint32_t prgSlotSize = getPRGSlotSize();
    uint32_t bankOffset = ((val * prgSlotSize) + offset) & (globalROM.PRGRomSize - 1);
    uint16_t cpuStart = 0x8000 + (slot * prgSlotSize);
    uint16_t cpuEnd = cpuStart + prgSlotSize - 1;

    mapCPUMemory(cpuStart, cpuEnd, globalROM.ROM, bankOffset, false, cpuStart >> 8, false);
}
void MapperBase::setPRGSlot8(uint16_t slot, uint16_t val, uint32_t offset) {
    setPRGSlot4(slot, val, offset);
    setPRGSlot4(slot*2+1, val+4, offset);
}
void MapperBase::setPRGSlot4(uint16_t slot, uint16_t val, uint32_t offset) {
    setPRGSlot2(slot*2, val, offset);
    setPRGSlot2(slot*2+1, val+2, offset);
}
void MapperBase::setPRGSlot2(uint16_t slot, uint16_t val, uint32_t offset) {
    setPRGSlot(slot*2, val, offset);
    setPRGSlot(slot*2+1, val+1, offset);
}

uint8_t MapperBase::cpuRead(uint16_t addr) {
    if (!PRGPages[addr >> 8].ptr) {
        if (showDebugLogs) printf("WARNING: tried reading from unmapped CPU memory at address 0x%x\n", addr);
        return cpu.emulateOBus ? cpu.OpenBus : 0xff;
    }
    return cpu.setOpenBus(PRGPages[addr >> 8].ptr[addr & 0xFF]);
}
void MapperBase::cpuWrite(uint16_t addr, uint8_t value) {
    if (PRGPages[addr >> 8].write) {
        PRGPages[addr >> 8].ptr[addr & 0xFF] = value;
        if (PRGPages[addr >> 8].battery) {
            globalROM.mapper->saveSRAM(cpu.PrgRAM);
            if (showDebugLogs) {
                printf("Saved SRAM value 0x%02x\n", value);
            }
        }
    }
}

uint8_t MapperBase::ppuRead(uint16_t addr) {
    addr &= 0x1FFF;
    if (!CHRPages[addr >> 8].ptr) {
        if (showDebugLogs) printf("WARNING: tried reading from unmapped PPU memory at address 0x%x\n", addr);
        return 0xff;
    }

    return CHRPages[addr >> 8].ptr[addr & 0xFF];
}
void MapperBase::ppuWrite(uint16_t addr, uint8_t value) {
    addr &= 0x1FFF;
    if (CHRPages[addr >> 8].write) {
        CHRPages[addr >> 8].ptr[addr & 0xFF] = value;
    }
}

void MapperBase::mapCPUMemory(uint16_t start, uint16_t end, uint8_t* memory, uint32_t offset, bool writable, uint8_t pageNum, bool battery) {
    uint8_t page = pageNum;
    
    for (uint32_t addr = start; addr <= end; addr += 0x100) {
        PRGPages[page].ptr = memory + ((offset + (addr - start)) & (globalROM.PRGRomSize-1));
        PRGPages[page].write = writable;
        PRGPages[page].battery = battery;
        page++;
    }
}
void MapperBase::unmapCPUMemory(uint16_t start, uint16_t end, uint8_t pageNum) {
    uint8_t page = pageNum;
    
    for (uint32_t addr = start; addr <= end; addr += 0x100) {
        PRGPages[page].ptr = nullptr;
        PRGPages[page].write = false;
        PRGPages[page].battery = false;
        page++;
    }
}

void MapperBase::mapPPUMemory(uint16_t start, uint16_t end, uint8_t* memory, uint32_t offset, bool writable) {
    uint8_t page = start >> 8;

    for (uint32_t addr = start; addr <= end; addr += 0x100) {
        CHRPages[page & 0x1F].ptr = memory + ((offset + (addr - start)) & (globalROM.CHRRomSize - 1));
        CHRPages[page & 0x1F].write = writable;
        page++;
    }
}

void MapperBase::saveSRAM(uint8_t *mem) {
    FILE* f = fopen(("saves/"+globalROM.Name+".sav").c_str(), "wb");
    if (!f) return;
    fwrite(mem, 1, 0x2000, f);
    fclose(f);
}
void MapperBase::loadSRAM(uint8_t *mem) {
    FILE* f = fopen(("saves/"+globalROM.Name+".sav").c_str(), "rb");
    if (!f) return;
    fread(mem, 1, 0x2000, f);
    fclose(f);
}