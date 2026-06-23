#pragma once

#include "../console.hpp"

class NESConsole : public Console {
public:
    ConsoleType getConsoleType(void) override { return ConsoleType::NES; }
    bool loadGame(const std::string &filename) override;
    void runFrame(void) override;
    int getDisplayWidth(void) override;
    int getDisplayHeight(void) override;
    void handleController(int id, int key, bool pressed) override;
    double getAudioOutput(void) override;
    QImage *getOutputImage(void) override;
    void setVideoFilter(int filter) override;
    void writeSave(void) override;
    void loadSave(void) override;
    void init(void) override;
    void pause(void) override;
    void reset(void) override;
};
