#pragma once

#include <cstdint>
#include <string>

#include <QImage>

enum class ConsoleRegion {
    NTSC,
    PAL,
    DENDY,
};

class ROMImage {
public:
    ConsoleRegion Region = ConsoleRegion::NTSC;
    std::string Name = "";

    virtual ~ROMImage() { }
    virtual bool load(const std::string &file) { (void)file; return false; }
};

enum class ConsoleType {
    UNKNOWN,
    NES,
    GAMEBOY,
};

class Console {
public:
    ROMImage *rom = nullptr;

    virtual ~Console() { if (rom) { delete rom; rom = nullptr; } }
    virtual ConsoleType getConsoleType(void) { return ConsoleType::UNKNOWN; }
    virtual bool loadGame(const std::string &filename) { (void)filename; return false; }
    virtual void runFrame(void) {}
    virtual int getDisplayWidth(void) { return 0; }
    virtual int getDisplayHeight(void) { return 0; }
    virtual void handleController(int id, int key, bool pressed) { (void)id; (void)key; (void)pressed; }
    virtual double getAudioOutput(void) { return 0.0; }
    virtual QImage *getOutputImage(void) { return nullptr; };
    virtual void writeSave(void) {}
    virtual void loadSave(void) {}
    virtual void init(void) {}
    virtual void pause(void) {}
    virtual void reset(void) {}
};

extern Console *emuConsole;
ROMImage *getRom(void);