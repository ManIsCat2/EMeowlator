#pragma once

#include "../console.hpp"

class NESConsole : public Console {
    bool loadGame(const std::string &file) override;
    void runFrame(void) override;
    void handleController(int id, int qtKey, bool pressed) override;
    double getAudioOutput(void) override;
    void writeSave(void) override;
    void loadSave(void) override;
    void init(void) override;
    void reset(void) override;
};
