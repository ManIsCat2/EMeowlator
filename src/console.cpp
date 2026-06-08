#include "console.hpp"

Console *emuConsole = nullptr;

ROMImage *getRom(void) {
    return emuConsole->rom;
}