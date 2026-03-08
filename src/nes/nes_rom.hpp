#pragma once

enum class HeaderVersion {
    NES2_0,
    INES,
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
    uint8_t Header[8];
    bool hasBattery = false;
    HeaderVersion Version = HeaderVersion::INES;
    uint8_t *ROM = nullptr;
    std::string Name = "";
    uint16_t ResetVec = 0;
    size_t PRGRomSize = 0;
    size_t CHRRomSize = 0;
    uint16_t MapperID = 0;
    uint16_t SubMapperID = 0;
    MapperBase *mapper = nullptr;

    MapperBase *GetMapper(void);
    bool LoadNES(const std::string &filename);
};