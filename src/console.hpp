#pragma once

#include <cstdint>
#include <string>

enum class ConsoleRegion {
    NTSC,
    PAL,
    DENDY,
};

class ROMImage {
public:
    ConsoleRegion Region = ConsoleRegion::NTSC;
    uint8_t *data = nullptr;

    virtual ~ROMImage() { }
    virtual bool load(const std::string &file) { (void)file; return false; }
};

class Console {
public:
    ROMImage *rom = nullptr;

    virtual ~Console() { if (rom) { delete rom; rom = nullptr; } }
    virtual bool loadGame(const std::string &file) { (void)file; return false; }
    virtual void runFrame(void) {}
    virtual void handleController(int id, int qtKey, bool pressed) { (void)id; (void)qtKey; (void)pressed; }
    virtual double getAudioOutput(void) { return 0.0; }
    virtual void writeSave(void) {}
    virtual void loadSave(void) {}
    virtual void init(void) {}
    virtual void reset(void) {}
};

extern Console *emuConsole;
ROMImage *getRom(void);