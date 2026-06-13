#include "gb_console.hpp"
#include "../nes/nes_controller.hpp"
#include "gb_cpu.hpp"
#include "gb_ppu.hpp"
#include "gb_rom.hpp"

#define CYCLES_PER_FRAME 70224

bool GBConsole::loadGame(const std::string &file) {
    if (!rom) {
        rom = new GbROM;
    }
    return rom->load(file);
}

void GBConsole::runFrame(void) {
    gbCpu.run(CYCLES_PER_FRAME);
}

int GBConsole::getDisplayWidth(void) {
    return 160*1.5;
}
int GBConsole::getDisplayHeight(void) {
    return 144*1.5;
}

void GBConsole::handleController(int id, int key, bool pressed) {

}

double GBConsole::getAudioOutput(void) {
    return 0.0;
}

QImage *GBConsole::getOutputImage(void) {
    return gbPpu.rawOutputImage;
}

void GBConsole::loadSave(void) {
    
}
void GBConsole::writeSave(void) {
    
}

void GBConsole::init(void) {
    
}

void GBConsole::reset(void) {
    gbCpu.reset();
}

GbROM *getGBRom(void) {
    return (GbROM*)getRom();
}