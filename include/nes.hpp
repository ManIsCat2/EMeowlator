#pragma once

#define NES_WIDTH  256
#define NES_HEIGHT 240

#define RAM_SIZE 0x800
#define RAM_MIRRORED_SIZE 0x2000
#define VRAM_SIZE 0x800
#define VRAM_MIRRORED_SIZE 0x4000
#define PALRAM_SIZE 0x20

#define A_BUTTON        (1 << 0)
#define B_BUTTON        (1 << 1)
#define SELECT_BUTTON   (1 << 2)
#define START_BUTTON    (1 << 3)
#define DPAD_UP        (1 << 4)
#define DPAD_DOWN      (1 << 5)
#define DPAD_LEFT      (1 << 6)
#define DPAD_RIGHT     (1 << 7)

#define CYCLES_PER_FRAME (1789773.0 / 60.0)

#define VRAM_FIY 0x7000 //0b111000000000000, fine Y
#define VRAM_X_NT 0x0400 //0b000010000000000, X nametable
#define VRAM_Y_NT 0x0800 //0b000100000000000, Y nametable
#define VRAM_COY 0x03E0 //0b000001111100000, coarse Y
#define VRAM_COX 0x001F //0b000000000011111, coarse X

#define UNUSED __attribute__((unused))

#include "debug.hpp"