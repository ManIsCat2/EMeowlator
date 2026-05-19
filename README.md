# MeowNES

MeowNES is a NES Emulator made by a Cat.

Uses nes_ntsc for the NTSC Video Filter which I got from Mesen2, couldn't find the original source. It is modified to be used with MeowNES though.

Supported mappers:
- NROM (Mapper 0)
- MMC1 (Mapper 1)
- UxROM (Mapper 2)
- CNROM (Mapper 3)
- MMC3 (Mapper 4)
- MMC5 (Mapper 5) (WIP)
- MMC2 (Mapper 9)
- Namco163 / Namco129 (Mapper 19)
- Nina001 (Mapper 34, SubMapper 1)
- BNROM (Mapper 34, SubMapper 2)
- X in 1 (Mapper 62)
- SunSoft FME-7 (Mapper 69)
- SL12 (?) (Mapper 116)
- J.Y. Company ASIC (Mapper 90, 209 and 211) (WIP)

# Building

## Step 1: Install Dependencies

### Ubuntu / Debian
```bash
sudo apt install qt6-base-dev libsdl2-dev build-essential clang
```

### Arch Linux
```bash
sudo pacman -S qt6-base sdl2 base-devel clang
```

### Fedora
```bash
sudo dnf install qt6-qtbase-devel SDL2-devel make clang
```

### Windows (MinGW)
```bash
pacman -S mingw-w64-x86_64-SDL2 mingw-w64-x86_64-qt6 mingw-w64-x86_64-toolchain
```

## Step 2: Clone the Repository
```bash
git clone https://github.com/ManIsCat2/MeowNES
```

## Step 3: Compile
```bash
cd path/to/MeowNES
make -j$(nproc)
```

The built executable will be located in the `build/` folder inside MeowNES.