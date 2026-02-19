#include "nes_rom.hpp"

MapperBase *NesROM::GetMapper(void) {
    switch (MapperID) {
        case 0: return new NROM();
        case 1: return new MMC1();
        case 2: return new UxROM();
        case 3: return new CNROM();
        case 4: return new MMC3();
        case 9: return new MMC2();
        case 19: return new Namco163();
        case 34: 
            switch(SubMapperID) {
                case 0: 
                    if (CHRRomSize > 0) {
                        return new NINA01();
                    } else {
                        return new BNROM();
                    }
                case 1: return new NINA01();
                case 2: default: return new BNROM();
            }
			break;
        case 69: return new SunSoftFME7();
        case 90: case 209: case 211: return new JyCompany();
        //hope for the best
        default: {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING , "Warning", ("Mapper " + std::to_string(MapperID) + " is Unimplemented, Using MMC1 instead.").c_str(), NULL);
            return new MMC1();
        }
    }
}

bool NesROM::LoadNES(const std::string &filename) {
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
        std::cerr << "Invalid NES header\n";
        return false;
    }

    std::memcpy(Header, data.data(), 8);

    ppu.Mirroring = (Header[6] & 1) ? MirrorMode::VERTICAL :  MirrorMode::HORIZONTAL;

    uint8_t prgPages = data[4];
    uint8_t chrPages = data[5];
    uint8_t flags6 = data[6];
    uint8_t flags7 = data[7];
    uint8_t flags8 = data[8];
    uint8_t flags9 = data[9];

    bool hasTrainer = (flags6 & 0x04) != 0;

    if ((flags7 & 0x0C) == 0x08) {
        Version = HeaderVersion::NES2_0;
    } else {
        Version = HeaderVersion::INES;
    }

    if (Version == HeaderVersion::NES2_0) {
        MapperID = ((flags8 & 0x0F) << 8) | (flags7 & 0xF0) | (flags6 >> 4);
        SubMapperID = flags8 >> 8;

        uint16_t prgNewVal = flags9 & 0x0F;
        uint16_t chrNewVal = flags9 >> 4;

        PRGRomSize = ((prgNewVal << 8) | prgPages) * 16 * 1024;
        CHRRomSize = ((chrNewVal << 8) | chrPages) * 8 * 1024;
    } else {
        MapperID = (flags7 & 0xF0) | (flags6 >> 4);
        SubMapperID = 0;

        PRGRomSize = size_t(prgPages) * 16 * 1024;
        CHRRomSize = size_t(chrPages) * 8 * 1024;
    }

    if (ROM) { delete[] ROM; ROM = nullptr; }
    ROM = new uint8_t[PRGRomSize];

    size_t offset = 16;
    if (hasTrainer) {
        if (data.size() < offset + 512) {
            std::cerr << "ROM too small\n";
            return false;
        }
        offset += 512;
    }
        
    if (!prgPages) {
        std::cerr << "ROM has zero PRG pages.\n";
        return false;
    }

    std::memcpy(ROM, &data[offset], PRGRomSize);
    offset += PRGRomSize;
        
    if (chrPages == 0) {
        uint8_t zeros[0x2000] = {};
        ppu.LoadCHRROM(zeros, 0x2000);
    } else {
        ppu.LoadCHRROM(&data[offset], CHRRomSize);
    }
    offset += CHRRomSize;
        
    if (mapper) { delete mapper; mapper = nullptr; }
        
    mapper = GetMapper();
    mapper->subMapper = SubMapperID;
    mapper->initialize();
    return true;
}