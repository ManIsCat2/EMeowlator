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
    nesCpu.run(speed);
}

int NESConsole::getDisplayWidth(void) {
    return 256;
}
int NESConsole::getDisplayHeight(void) {
    return 240;
}

void NESConsole::handleController(int id, int key, bool pressed) {
    Controller &c = controllers[id];
    if (key == nesKeyBinds[0].key) pressed ? c.state|=A_BUTTON : c.state&=~A_BUTTON;
    if (key == nesKeyBinds[1].key) pressed ? c.state|=B_BUTTON : c.state&=~B_BUTTON;
    if (key == nesKeyBinds[2].key) pressed ? c.state|=DPAD_UP : c.state&=~DPAD_UP;
    if (key == nesKeyBinds[3].key) pressed ? c.state|=DPAD_DOWN : c.state&=~DPAD_DOWN;
    if (key == nesKeyBinds[4].key) pressed ? c.state|=DPAD_LEFT : c.state&=~DPAD_LEFT;
    if (key == nesKeyBinds[5].key) pressed ? c.state|=DPAD_RIGHT : c.state&=~DPAD_RIGHT;
    if (key == nesKeyBinds[6].key) pressed ? c.state|=START_BUTTON : c.state&=~START_BUTTON;
    if (key == nesKeyBinds[7].key) pressed ? c.state|=SELECT_BUTTON : c.state&=~SELECT_BUTTON;
}

double NESConsole::getAudioOutput(void) {
    return nesApu.getOutputSample();
}

QImage *NESConsole::getOutputImage(void) {
    if (nesPpu.filtering == VideoFilter::NTSC) {
        nesPpu.vfilter->initialize();
        return nesPpu.filteredOutputImage;
    } else {
       return nesPpu.rawOutputImage;
    }
}

void NESConsole::loadSave(void) {
    getNESRom()->mapper->loadSRAM();
}
void NESConsole::writeSave(void) {
    getNESRom()->mapper->saveSRAM();
}

void NESConsole::init(void) {
    nesPpu.Init();
}

void NESConsole::pause(void) {
    nesCpu.paused = !nesCpu.paused;
}

void NESConsole::reset(void) {
    nesCpu.reset();
}

NesROM *getNESRom(void) {
    return (NesROM*)getRom();
}