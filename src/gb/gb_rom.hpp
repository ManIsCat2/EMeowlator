#pragma once

#include "gb_console.hpp"
#include "mbcs/mbc_base.hpp"
#include <string>
#include <cstdint>

class GbROM : public ROMImage {
public:
    GbROM();
    ~GbROM();

    uint8_t Header[80];
    uint8_t *ROM = nullptr;
    size_t RomSize = 0;
    uint8_t cartType = 0;
    MBCBase *mbc = nullptr;
    uint32_t ramSize = 0;
    
    std::string Title = ""; 

    bool load(const std::string &file) override;
};