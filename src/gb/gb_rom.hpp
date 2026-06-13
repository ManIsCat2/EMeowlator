#pragma once

#include "gb_console.hpp"
#include <string>
#include <cstdint>

class GbROM : public ROMImage {
public:
    GbROM();
    ~GbROM();

    uint8_t Header[80];
    uint8_t *ROM = nullptr;
    size_t RomSize = 0;
    
    std::string Name = "";  
    std::string Title = ""; 

    bool load(const std::string &file) override;
};