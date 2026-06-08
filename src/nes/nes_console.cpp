#include "nes_console.hpp"
#include "nes_cpu.hpp"
#include "nes_apu.hpp"
#include "nes_rom.hpp"

#define A_BUTTON       (1 << 0)
#define B_BUTTON       (1 << 1)
#define SELECT_BUTTON  (1 << 2)
#define START_BUTTON   (1 << 3)
#define DPAD_UP        (1 << 4)
#define DPAD_DOWN      (1 << 5)
#define DPAD_LEFT      (1 << 6)
#define DPAD_RIGHT     (1 << 7)

#define CYCLES_PER_FRAME_NTSC (1789773.0 / 60.0)
#define CYCLES_PER_FRAME_PAL (1662607.0 / 50.0)

bool NESConsole::loadGame(const std::string &file) {
    if (!rom) {
        rom = new NesROM;
    }
    return rom->load(file);
}

void NESConsole::runFrame(void) {
    uint32_t speed = CYCLES_PER_FRAME_NTSC;
    if (rom->Region == ConsoleRegion::PAL) {
        speed = CYCLES_PER_FRAME_PAL;
    }
    cpu.run(speed);
}

void NESConsole::handleController(int id, int qtKey, bool pressed) {
    Controller &c = controllers[id];
    if (qtKey == nesKeyBinds[0].key) pressed ? c.state|=A_BUTTON : c.state&=~A_BUTTON;
    if (qtKey == nesKeyBinds[1].key) pressed ? c.state|=B_BUTTON : c.state&=~B_BUTTON;
    if (qtKey == nesKeyBinds[2].key) pressed ? c.state|=DPAD_UP : c.state&=~DPAD_UP;
    if (qtKey == nesKeyBinds[3].key) pressed ? c.state|=DPAD_DOWN : c.state&=~DPAD_DOWN;
    if (qtKey == nesKeyBinds[4].key) pressed ? c.state|=DPAD_LEFT : c.state&=~DPAD_LEFT;
    if (qtKey == nesKeyBinds[5].key) pressed ? c.state|=DPAD_RIGHT : c.state&=~DPAD_RIGHT;
    if (qtKey == nesKeyBinds[6].key) pressed ? c.state|=START_BUTTON : c.state&=~START_BUTTON;
    if (qtKey == nesKeyBinds[7].key) pressed ? c.state|=SELECT_BUTTON : c.state&=~SELECT_BUTTON;
}

double NESConsole::getAudioOutput(void) {
    return apu.getOutputSample();
}

void NESConsole::loadSave(void) {
    getNESRom()->mapper->loadSRAM();
}
void NESConsole::writeSave(void) {
    getNESRom()->mapper->saveSRAM();
}

void NESConsole::init(void) {

}

void NESConsole::reset(void) {
    cpu.reset();
}

NesROM *getNESRom(void) {
    return (NesROM*)getRom();
}