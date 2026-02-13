#pragma once

#include <cstdint>
#include <iostream>
#include <array>
#include <string>
#include <fstream>
#include <vector>
#include <cstring>
#include <filesystem>

#include <SDL2/SDL.h>

#include "nes.hpp"
#include "nes_cpu.hpp"
#include "nes_controller.hpp"

#include "mappers/mappers.hpp"

#include "nes_rom.hpp"

extern bool romIsLoaded;
extern NesROM globalROM;