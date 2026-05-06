#pragma once

#include <cstdint>

struct PulseChannel {
    bool enable = false;
    uint8_t duty = 0;
    uint8_t dutySeq = 0;
    uint16_t timer = 0;
    uint16_t timerReload = 0;
    uint8_t lengthCounter = 0;
    bool lengthHalt = false;
    bool constantVolume = false;
    uint8_t volume = 0;
    bool envStart = false;
    uint8_t envVol = 0;
    uint8_t envDivider = 0;
};

struct TriangleChannel {
    bool enable = false;
    bool lengthHalt = false;
    uint8_t linearCounter = 0;
    uint8_t linearReload = 0;
    bool linearReloadFlag = false;
    uint16_t timer = 0;
    uint16_t timerReload = 0;
    uint8_t lengthCounter = 0;
    uint8_t dutySeq = 0;
};

struct NoiseChannel {
    bool enable = false;
    bool lengthHalt = false;
    bool constantVolume = false;
    uint8_t volume = 0;
    uint16_t timer = 0;
    uint16_t timerReload = 0;
    uint8_t lengthCounter = 0;
    uint16_t shiftRegister = 1;
    bool mode = false;
    bool envStart = false;
    uint8_t envVol = 0;
    uint8_t envDivider = 0;
};

struct DMCChannel {
    bool enable = false;
    uint16_t lengthCounter = 0;
    uint16_t reloadLength = 1;
    uint16_t timer = 0;
    uint16_t timerReload = 428;
    uint8_t bitCounter = 0;
    bool loop = false;
};

class APU {
public:
    APU();
    ~APU();

    void write(uint16_t addr, uint8_t data);
    uint8_t read(uint16_t addr);
    
    void step();
    void reset();

    bool IRQPending = false;
    bool DMCIrq = false; 
    bool DMCIrqEnable = false;

    float pulse1Volume = 50.0f;
    float pulse2Volume = 50.0f;
    float triangleVolume = 50.0f;
    float noiseVolume = 50.0f;
private:
    PulseChannel pulse1;
    PulseChannel pulse2;
    TriangleChannel triangle;
    NoiseChannel noise;       
    DMCChannel dmc; 

    uint32_t clockCounter = 0;
    uint32_t frameCounter = 0;
    uint8_t frameMode = 0;
    bool IRQInhibit = false;
    
    int frameCounterResetDelay = 0;
    uint8_t delayedFrameMode = 0;
    
    void clockEnvelopes();
    void clockLengths();
};

extern APU apu;