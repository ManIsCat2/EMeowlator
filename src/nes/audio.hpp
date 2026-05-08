#pragma once

#include <SDL2/SDL.h>
#include <queue>
#include <mutex>

class Audio {
public:
    std::queue<double> buffer;
    std::mutex mutex;
    int sampleRate = 44100;
    double cycleCounter = 0.0;

    SDL_AudioDeviceID device = 0;
    void init();
    void close();
    void advance();
    void pushSample();
};

extern Audio audioSystem;