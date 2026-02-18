#pragma once
#include "../nes_ppu.hpp"
#include <stdint.h>

class MapperBase {
public:
    int CHRBankOffset[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    uint16_t subMapper = 0;

    struct MemPage {
        uint8_t *ptr = nullptr;
        bool write = false;
    };
    struct MemPage PRGPages[256];
    struct MemPage CHRPages[256];

    virtual ~MapperBase() = default;
    virtual uint8_t cpuRead(uint16_t addr);
    virtual void cpuWrite(uint16_t addr, uint8_t value);
    virtual uint8_t ppuRead(uint16_t addr);
    virtual void ppuWrite(uint16_t addr, uint8_t value);
    virtual const char *getName(void) { return ""; }
    virtual void reset() {}
    void initialize() {

        reset();
    }

    virtual uint16_t getCHRSlotSize() {
        return 0x2000;
    }
	virtual uint16_t getPRGSlotSize() {
        return 0x4000;
    }

    void setCHRSlot(uint16_t slot, uint16_t val, uint32_t offset=0);
    void setCHRSlot8(uint16_t slot, uint16_t val, uint32_t offset=0);
    void setCHRSlot4(uint16_t slot, uint16_t val, uint32_t offset=0);
    void setCHRSlot2(uint16_t slot, uint16_t val, uint32_t offset=0);

    void setPRGSlot(uint16_t slot, uint16_t val, uint32_t offset=0);
    void setPRGSlot8(uint16_t slot, uint16_t val, uint32_t offset=0);
    void setPRGSlot4(uint16_t slot, uint16_t val, uint32_t offset=0);
    void setPRGSlot2(uint16_t slot, uint16_t val, uint32_t offset=0);

    void mapCPUMemory(uint16_t start, uint16_t end, uint8_t* memory, uint32_t offset, bool writable, uint8_t pageNum);
    void mapPPUMemory(uint16_t start, uint16_t end, uint8_t* memory, uint32_t offset, bool writable);
};
