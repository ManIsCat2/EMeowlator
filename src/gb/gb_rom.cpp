#include "gb_rom.hpp"
#include "mbcs/mbcs.hpp"
#include "../main.hpp"
#include <fstream>
#include <iostream>

GbROM::GbROM() {

}

GbROM::~GbROM() {
    delete[] ROM;
    delete mbc;
}

bool GbROM::load(const std::string &filename) {
    std::filesystem::path Path(filename);
    Name = Path.filename().string();

    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return false;
    }

    RomSize = file.tellg();
    if (RomSize < 0x0150) {
        return false;
    }

    ROM = new uint8_t[RomSize];

    file.seekg(0, std::ios::beg);
    file.read((char*)ROM, RomSize);
    file.close();

    for (int i = 0; i < 80; i++) {
        Header[i] = ROM[0x0100 + i];
    }

    Title = "";
    for (uint16_t addr = 0x0134; addr <= 0x0143; addr++) {
        char c = (char)ROM[addr];
        if (c == '\0') break;
        Title += c;
    }

    cartType = ROM[0x0147];

    switch (ROM[0x0149]) {
        case 0x00: ramSize = 0; break;
        case 0x01: ramSize = 2 * 1024; break;
        case 0x02: ramSize = 8 * 1024; break;
        case 0x03: ramSize = 32 * 1024; break;
        case 0x04: ramSize = 128 * 1024; break;
        case 0x05: ramSize = 64 * 1024; break;
        default: {
            ramSize = 0x2000;
            break;
        }
    }
    switch (cartType) {
        case 0x00:
            mbc = new MBC0();
            break;

        case 0x01:
        case 0x02:
        case 0x03:
            mbc = new MBC1();
            break;

        case 0x05:
        case 0x06:
            break;

        case 0x0F:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
            break;

        case 0x19:
        case 0x1A:
        case 0x1B:
        case 0x1C:
        case 0x1D:
        case 0x1E:
            break;
    }

    if (!mbc) {
        DebugPrintLog("ROM", "Unimplemented mapper: %u, failed to open ROM", cartType);
        QMessageBox::critical((QMainWindow*)globalQTWin, "Error", ("Mapper " + std::to_string(cartType) + " is unimplemented, failed to open ROM").c_str());
        return false;
    }
    mbc->initialize();

    DebugPrintLog("ROM", "Loaded GameBoy ROM '%s'", Name.c_str());

    return true;
}