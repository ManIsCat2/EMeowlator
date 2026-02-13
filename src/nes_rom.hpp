#include <SDL2/SDL.h>
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
    uint8_t ROM[0x200000];
    std::string Name = "";
    size_t PRGRomSize = 0;
    size_t CHRRomSize = 0;
    uint16_t MapperID = 0;
    MapperBase *mapper = nullptr;

    MapperBase *GetMapper(void);
    bool LoadNES(const std::string &filename);
};