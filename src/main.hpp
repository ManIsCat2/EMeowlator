#pragma once

#include <cstdint>
#include <iostream>
#include <array>
#include <string>
#include <fstream>
#include <vector>
#include <cstring>
#include <filesystem>

#include "nes.hpp"
#include "nes_cpu.hpp"
#include "nes_controller.hpp"

#include "mappers/mappers.hpp"

extern bool romIsLoaded;
class NesROM;
extern NesROM globalROM;

class NesROM {
public:
    uint8_t Header[8];
    uint8_t ROM[0x200000];
    std::string Name = "";
    size_t PRGRomSize = 0;
    size_t CHRRomSize = 0;
    uint16_t MapperID = 0;
    MapperBase *mapper = nullptr;

    MapperBase *GetMapper(void) {
        switch (MapperID) {
            case 0: return new NROM();
            case 1: return new MMC1();
            case 2: return new UxROM();
            case 3: return new CNROM();
            case 9: return new MMC2();
            //hope for the best
            default: return new MMC1();
        }
    }

    bool LoadNES(const std::string &filename) {
        std::ifstream rom(filename, std::ios::binary | std::ios::ate);
        if (!rom) {
            std::cerr << "Failed to open ROM: " << filename << "\n";
            return false;
        }

        std::filesystem::path Path(filename);
        Name = Path.filename().string();

        std::streamsize fsize = rom.tellg();
        if (fsize < 16) {
            std::cerr << "ROM too small\n";
            return false;
        }
        rom.seekg(0, std::ios::beg);

        std::vector<uint8_t> data((size_t)fsize);
        if (!rom.read(reinterpret_cast<char*>(data.data()), fsize)) {
            std::cerr << "Failed to read ROM\n";
            return false;
        }

        // header
        if (data.size() < 16 || data[0] != 'N' || data[1] != 'E' || data[2] != 'S' || data[3] != 0x1A) {
            std::cerr << "Invalid iNES header\n";
            return false;
        }

        std::memcpy(Header, data.data(), 8);

        uint8_t prgPages = data[4];
        uint8_t chrPages = data[5];
        uint8_t flags6 = data[6];
        uint8_t flags7 = data[7];

        bool hasTrainer = (flags6 & 0x04) != 0;

        MapperID = (flags7 & 0xF0) | (flags6 >> 4);
        size_t offset = 16;
        if (hasTrainer) {
            if (data.size() < offset + 512) {
                std::cerr << "ROM too small\n";
                return false;
            }
            offset += 512;
        }

        PRGRomSize = size_t(prgPages) * 16 * 1024;

        if (!prgPages) {
            std::cerr << "ROM has zero PRG pages.\n";
            return false;
        }
        if (MapperID) {
            std::memcpy(&ROM[0], &data[offset], PRGRomSize);
        } else {
            if (prgPages == 1) {
                std::memcpy(&ROM[0], &data[offset], 0x4000);
                std::memcpy(&ROM[0x4000], &data[offset], 0x4000);
            } else {
                std::memcpy(&ROM[0], &data[offset], 0x4000);
                std::memcpy(&ROM[0x4000], &data[offset + 0x4000], 0x4000);
            }
        }
        offset += PRGRomSize;

        CHRRomSize = size_t(chrPages) * 8 * 1024;
        if (chrPages == 0) {
            uint8_t zeros[0x2000] = {};
            ppu.LoadCHRROM(zeros, 0x2000);
        } else {
            ppu.LoadCHRROM(&data[offset], CHRRomSize);
        }
        offset += CHRRomSize;

        if (mapper) { delete mapper; mapper = nullptr; }

        mapper = GetMapper();

        mapper->reset();
        return true;
    }
};