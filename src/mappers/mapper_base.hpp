#pragma once
#include "../nes_ppu.hpp"
#include <stdint.h>

class MapperBase {
public:
    int PRGBankOffset[4] = {0, 0, 0, 0};
    int CHRBankOffset[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    virtual ~MapperBase() = default;
    virtual uint8_t cpuRead(uint16_t addr) { (void)addr; return 0; }
    virtual void cpuWrite(uint16_t addr, uint8_t value) { (void)addr; (void)value; }
    virtual uint8_t ppuRead(uint16_t addr) { (void)addr; return 0; }
    virtual const char *getName(void) { return ""; }
    virtual void reset() {}

    virtual uint16_t getCHRSlotSize() {
        return 0x2000;
    }

    void setCHRSlot(uint16_t slot, uint16_t val, uint32_t offset=0) {
        uint16_t chrSlotSize = getCHRSlotSize();
        CHRBankOffset[slot] = ((val * chrSlotSize) + offset) % ppu.ChrData.size();
    }

    void setCHRSlot8(uint16_t slot, uint16_t val, uint32_t offset=0) {
        setCHRSlot4(slot, val, offset);
        setCHRSlot4(slot*2+1, val+4, offset);
    }

    void setCHRSlot4(uint16_t slot, uint16_t val, uint32_t offset=0) {
        setCHRSlot2(slot*2, val, offset);
        setCHRSlot2(slot*2+1, val+2, offset);
    }

    void setCHRSlot2(uint16_t slot, uint16_t val, uint32_t offset=0) {
        setCHRSlot(slot*2, val, offset);
        setCHRSlot(slot*2+1, val+1, offset);
    }
};
