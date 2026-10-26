#pragma once

#include <SDL3/SDL.h>
#include <queue>
#include <mutex>

class Audio {
public:
    std::queue<double> buffer;
    std::mutex mutex;
    int sampleRate = 44100;
    double cycleCounter = 0.0;

    SDL_AudioStream* stream = nullptr;
    void init();
    void close();
    void advance(uint32_t cycles);
    void pushSample();
};

extern Audio audioSystem;