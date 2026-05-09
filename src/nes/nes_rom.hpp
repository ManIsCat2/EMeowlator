#pragma once

enum class HeaderVersion {
    NES2_0,
    INES,
};

enum class ConsoleRegion {
    NTSC,
    PAL,
    DENDY,
};

#include <cstdint>
#include <iostream>
#include <array>
#include <string>
#include <fstream>
#include <vector>
#include <cstring>
#include <filesystem>

#include "mappers/mappers.hpp"
#include "nes_ppu.hpp"

class NesROM {
public:
    NesROM();
    ~NesROM();
    uint8_t Header[16];
    bool hasBattery = false;
    HeaderVersion Version = HeaderVersion::INES;
    ConsoleRegion Region = ConsoleRegion::NTSC;
    uint8_t *ROM = nullptr;
    std::string Name = "";
    uint16_t ResetVec = 0;
    MirrorMode Mirroring = MirrorMode::HORIZONTAL;
    uint8_t PRGNumPages = 0;
    uint8_t CHRNumPages = 0;
    size_t PRGRomSize = 0;
    size_t CHRRomSize = 0;
    uint16_t MapperID = 0;
    uint16_t SubMapperID = 0;
    MapperBase *mapper = nullptr;

    MapperBase *GetMapper(uint16_t id, uint16_t subId);
    ConsoleRegion GetRegion(void);
    bool LoadNES(const std::string &filename);
};