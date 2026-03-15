#include "mapper_base.hpp"
#include "../nes_rom.hpp"
#include "../../main.hpp"

MapperBase::~MapperBase() {
    if (SRAM) delete[] SRAM;
}

void MapperBase::setCHRPages(uint16_t page, uint16_t val, enum BankSize size, uint32_t offset) {
    switch (size) {
        case BANK_1K: {
            uint32_t chrSlotSize = getCHRPageSize();
            uint32_t bankOffset = ((val * chrSlotSize) + offset) & (globalROM.CHRRomSize - 1);
            uint16_t ppuStart = page * chrSlotSize;
            uint16_t ppuEnd = ppuStart + chrSlotSize - 1;
            mapPPUMemory(ppuStart, ppuEnd, ppu.ChrData.data(), bankOffset, globalROM.CHRRomSize == 0);
            break;
        }
        case BANK_2K:
            setCHRPages(page * 2, val, BANK_1K, offset);
            setCHRPages(page * 2 + 1, val + 1, BANK_1K, offset);
            break;
        case BANK_4K:
            setCHRPages(page * 2, val, BANK_2K, offset);
            setCHRPages(page * 2 + 1, val + 2, BANK_2K, offset);
            break;
        case BANK_8K:
            setCHRPages(page * 2, val, BANK_4K, offset);
            setCHRPages(page * 2 + 1, val + 4, BANK_4K, offset);
            break;
    }
}

void MapperBase::setPRGPage(uint16_t page, uint16_t val, uint32_t offset) {
    uint32_t prgSlotSize = getPRGPageSize();
    uint32_t bankOffset = ((val * prgSlotSize) + offset) & (globalROM.PRGRomSize - 1);
    uint16_t cpuStart = 0x8000 + (page * prgSlotSize);
    uint16_t cpuEnd = cpuStart + prgSlotSize - 1;

    mapCPUMemory(cpuStart, cpuEnd, globalROM.ROM, bankOffset, false, cpuStart >> 8);
}
void MapperBase::setPRGPage8(uint16_t page, uint16_t val, uint32_t offset) {
    setPRGPage4(page, val, offset);
    setPRGPage4(page*2+1, val+4, offset);
}
void MapperBase::setPRGPage4(uint16_t page, uint16_t val, uint32_t offset) {
    setPRGPage2(page*2, val, offset);
    setPRGPage2(page*2+1, val+2, offset);
}
void MapperBase::setPRGPage2(uint16_t page, uint16_t val, uint32_t offset) {
    setPRGPage(page*2, val, offset);
    setPRGPage(page*2+1, val+1, offset);
}

uint8_t MapperBase::cpuRead(uint16_t addr) {
    if (!PRGPages[addr >> 8].ptr) {
        //DebugPrintLog("MAPPER", "tried reading from unmapped CPU memory at address 0x%x", addr);
        return cpu.OpenBus;
    }
    return cpu.setOpenBus(PRGPages[addr >> 8].ptr[addr & 0xFF]);
}
void MapperBase::cpuWrite(uint16_t addr, uint8_t value) {
    if (PRGPages[addr >> 8].write) {
        PRGPages[addr >> 8].ptr[addr & 0xFF] = value;
    }
}

uint8_t MapperBase::readCHR(uint16_t addr) {
    addr &= 0x1FFF;
    if (!CHRPages[addr >> 8].ptr) {
        //DebugPrintLog("MAPPER", "tried reading from unmapped PPU memory at address 0x%x", addr);
        return 0xff;
    }

    return CHRPages[addr >> 8].ptr[addr & 0xFF];
}
void MapperBase::writeCHR(uint16_t addr, uint8_t value) {
    addr &= 0x1FFF;
    if (CHRPages[addr >> 8].write) {
        CHRPages[addr >> 8].ptr[addr & 0xFF] = value;
    }
}

uint8_t MapperBase::readVRAM(uint16_t addr) {
    if (ppu.Mirroring == MirrorMode::HORIZONTAL) {
        addr = (addr & 0x3FF) | (addr & 0x800) >> 1;
        //addr = (addr & 0x800) ? (addr - 0x400) : addr;
    } else {
        addr &= 0x7FF;
    }
    return ppu.VRAM[addr];
}
void MapperBase::writeVRAM(uint16_t addr, uint8_t value) {
    if (ppu.Mirroring == MirrorMode::HORIZONTAL) {
        //addr = (addr & 0x3FF) | (addr & 0x800) >> 1;
        addr = (addr & 0x800) ? (addr - 0x400) : addr;
    } else {
        addr &= 0x7FF;
    }
    ppu.VRAM[addr] = value;
}

void MapperBase::mapCPUMemory(uint16_t start, uint16_t end, uint8_t *memory, uint32_t offset, bool writable, uint8_t pageNum) {
    uint8_t page = pageNum;
    
    for (uint32_t addr = start; addr <= end; addr += 0x100) {
        PRGPages[page].ptr = memory + ((offset + (addr - start)) & (globalROM.PRGRomSize-1));
        PRGPages[page].write = writable;
        page++;
    }
}
void MapperBase::unmapCPUMemory(uint16_t start, uint16_t end, uint8_t pageNum) {
    uint8_t page = pageNum;
    
    for (uint32_t addr = start; addr <= end; addr += 0x100) {
        PRGPages[page].ptr = nullptr;
        PRGPages[page].write = false;
        page++;
    }
}

void MapperBase::mapPPUMemory(uint16_t start, uint16_t end, uint8_t *memory, uint32_t offset, bool writable) {
    uint8_t page = start >> 8;

    for (uint32_t addr = start; addr <= end; addr += 0x100) {
        CHRPages[page & 0x1F].ptr = memory + ((offset + (addr - start)) & (globalROM.CHRRomSize - 1));
        CHRPages[page & 0x1F].write = writable;
        page++;
    }
}

void MapperBase::saveSRAM() {
    if (!globalROM.hasBattery) return;
    std::string fname = ("saves/"+globalROM.Name+".sav");
    FILE* f = fopen(fname.c_str(), "wb");
    if (!f) return;
    fwrite(SRAM, 1, getSRAMSize(), f);
    fclose(f);
    DebugPrintLog("MAPPER", "Saved SRAM to %s", fname.c_str())
}
void MapperBase::loadSRAM() {
    if (!globalROM.hasBattery) return;
    std::string fname = ("saves/"+globalROM.Name+".sav");
    FILE* f = fopen(fname.c_str(), "rb");
    if (!f) return;
    fread(SRAM, 1, getSRAMSize(), f);
    fclose(f);
    DebugPrintLog("MAPPER", "Loaded SRAM from %s", fname.c_str())
}

void MapperBase::initialize() {
    if (globalROM.hasBattery) {
        //DebugPrintLog("MAPPER", "Loaded Save");
        SRAM = new uint8_t[getSRAMSize()];
        loadSRAM();
    }
    reset();
}