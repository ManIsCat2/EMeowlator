#include "mapper_base.hpp"
#include "../nes_rom.hpp"
#include "../main.hpp"

void MapperBase::setCHRSlot(uint16_t slot, uint16_t val, uint32_t offset) {
    uint16_t chrSlotSize = getCHRSlotSize();
    CHRBankOffset[slot] = ((val * chrSlotSize) + offset) % globalROM.CHRRomSize;
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
    uint32_t bankOffset = ((val * prgSlotSize) + offset) % globalROM.PRGRomSize;
    uint16_t cpuStart = 0x8000 + (slot * prgSlotSize);
    uint16_t cpuEnd = cpuStart + prgSlotSize - 1;

    mapCPUMemory(cpuStart, cpuEnd, globalROM.ROM, bankOffset, false, cpuStart >> 8);
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
        char error[512];
        sprintf(error, "Tried to read unmapped memory at address 0x%x", addr);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR , "Fatal error", error, NULL);
        romIsLoaded = false;
        cpu.reset();
        return 0xff;
    }
    return PRGPages[addr >> 8].ptr[addr & 0xFF];
}
void MapperBase::cpuWrite(uint16_t addr, uint8_t val) {
    if (PRGPages[addr >> 8].write) PRGPages[addr >> 8].ptr[addr & 0xFF] = val;
}

void MapperBase::mapCPUMemory(uint16_t start, uint16_t end, uint8_t* memory, uint32_t offset, bool writable, uint8_t pageNum) {
    uint8_t page = pageNum;
    
    for (uint32_t addr = start; addr <= end; addr += 0x100) {
        PRGPages[page].ptr = memory + ((offset + (addr - start)) & (globalROM.PRGRomSize-1));
        PRGPages[page].write = writable;
        page++;
    }
}
