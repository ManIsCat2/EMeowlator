#include "../gb_rom.hpp"
#include "../gb_cpu.hpp"
#include "mbc_base.hpp"

MBCBase::~MBCBase() {
    if (cartRAM) delete[] cartRAM;
}

uint8_t MBCBase::cpuRead(uint16_t addr) {
    if (!CPUPages[addr >> 8].ptr) {
        return cpu->dataBus;
    }
    return cpu->dataBus = (CPUPages[addr >> 8].ptr[addr & 0xFF]);
}
void MBCBase::cpuWrite(uint16_t addr, uint8_t value) {
    if (CPUPages[addr >> 8].write) {
        CPUPages[addr >> 8].ptr[addr & 0xFF] = value;
    }
}

void MBCBase::mapCPUMemory(uint16_t start, uint16_t end, uint8_t *memory, uint32_t offset, bool writable) {
    uint8_t page = start >> 8;
    
    for (uint32_t addr = start; addr <= end; addr += 0x100) {
        CPUPages[page].ptr = memory + ((offset + (addr - start)) & (getGBRom()->RomSize - 1));
        CPUPages[page].write = writable;
        page++;
    }
}
void MBCBase::unmapCPUMemory(uint16_t start, uint16_t end) {
    uint8_t page = start >> 8;
    
    for (uint32_t addr = start; addr <= end; addr += 0x100) {
        CPUPages[page].ptr = nullptr;
        CPUPages[page].write = false;
        page++;
    }
}

void MBCBase::initialize(void) {
    connectBus(&gbCpu, nullptr);

    bool ramSize = getGBRom()->ramSize;
    if (ramSize) {
        cartRAM = new uint8_t[ramSize];
    }
    reset();
}