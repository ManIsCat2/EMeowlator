#pragma once
#include "../nes_ppu.hpp"
#include <stdint.h>
#include <string>

class MapperBase {
public:
    uint16_t subMapper = 0;

    struct MemPage {
        uint8_t *ptr = nullptr;
        bool write = false;
        bool battery = false;
    };
    struct MemPage PRGPages[256];
    struct MemPage CHRPages[32];

    uint8_t PRGRam[0x2000];
    uint8_t *SRAM = nullptr;

    virtual ~MapperBase();
    virtual uint8_t cpuRead(uint16_t addr);
    virtual void cpuWrite(uint16_t addr, uint8_t value);
    virtual uint8_t ppuRead(uint16_t addr);
    virtual void ppuWrite(uint16_t addr, uint8_t value);
    virtual const char *getName(void) { return ""; }
    virtual void reset() {}
    void initialize();

    virtual uint16_t getCHRPageSize() {
        return 0x2000;
    }
	virtual uint16_t getPRGPageSize() {
        return 0x4000;
    }
    virtual uint16_t getSRAMSize() { 
        return 0x2000;
    }

    virtual void clockCPU(void) {}
    virtual void clockPPU(void) {}

    void setCHRPage(uint16_t page, uint16_t val, uint32_t offset=0);
    void setCHRPage8(uint16_t page, uint16_t val, uint32_t offset=0);
    void setCHRPage4(uint16_t page, uint16_t val, uint32_t offset=0);
    void setCHRPage2(uint16_t page, uint16_t val, uint32_t offset=0);

    void setPRGPage(uint16_t page, uint16_t val, uint32_t offset=0);
    void setPRGPage8(uint16_t page, uint16_t val, uint32_t offset=0);
    void setPRGPage4(uint16_t page, uint16_t val, uint32_t offset=0);
    void setPRGPage2(uint16_t page, uint16_t val, uint32_t offset=0);

    void mapCPUMemory(uint16_t start, uint16_t end, uint8_t* memory, uint32_t offset, bool writable, uint8_t pageNum, bool battery);
    void unmapCPUMemory(uint16_t start, uint16_t end, uint8_t pageNum);
    void mapPPUMemory(uint16_t start, uint16_t end, uint8_t* memory, uint32_t offset, bool writable);

    void saveSRAM();
    void loadSRAM();
};
