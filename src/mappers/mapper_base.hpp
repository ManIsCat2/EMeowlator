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
};
