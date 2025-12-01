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
#define STICK_UP        (1 << 4)
#define STICK_DOWN      (1 << 5)
#define STICK_LEFT      (1 << 6)
#define STICK_RIGHT     (1 << 7)