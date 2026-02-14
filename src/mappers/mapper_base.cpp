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
    uint16_t prgSlotSize = getPRGSlotSize();
    PRGBankOffset[slot] = ((val * prgSlotSize) + offset) % globalROM.PRGRomSize;
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