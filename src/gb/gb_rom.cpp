#include "gb_rom.hpp"
#include <fstream>
#include <iostream>

GbROM::GbROM() {

}

GbROM::~GbROM() {
    if (ROM != nullptr) {
        delete[] ROM;
        ROM = nullptr;
    }
}

bool GbROM::load(const std::string &filename) {
    Name = filename;

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

    DebugPrintLog("ROM", "Loaded GameBoy ROM '%s', Title: %s", Name.c_str(), Title.c_str());

    return true;
}