#pragma once

#include <cstdint>
#include "nes_bus.hpp"

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
    bool sweepEnable = false;
    uint8_t sweepPeriod = 0;
    bool sweepNegate = false;
    uint8_t sweepShift = 0;
    bool sweepReload = false;
    uint8_t sweepDivider = 0;
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
    bool loop = false;
    uint16_t timer = 0;
    uint16_t timerReload = 428;
    uint16_t currentAddress = 0;
    uint16_t sampleAddress = 0xC000;
    uint16_t currentLength = 0;
    uint16_t sampleLength = 1;
    uint8_t shiftRegister = 0;
    uint8_t bitsRemaining = 8;
    uint8_t sampleBuffer = 0;
    bool sampleBufferEmpty = true;
    uint8_t outputLevel = 0;
    bool silence = true;
};

class NesAPU : public HasNESBus {
public:
    NesAPU();
    ~NesAPU();

    void write(uint16_t addr, uint8_t data);
    uint8_t read(uint16_t addr);
    
    void step();
    void reset();

    bool IRQPending = false;
    bool DMCIrqPending = false; 
    bool DMCIrqEnable = false;

    float pulse1Volume = 50.0f;
    float pulse2Volume = 50.0f;
    float triangleVolume = 50.0f;
    float noiseVolume = 50.0f;
    float dmcVolume = 50.0f;
    float masterVolume = 50.0f;
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
    double getOutputSample();
private:
    bool pulseSweepMuted(PulseChannel &p, bool isPulse1);
    void clockSweep(PulseChannel &p, bool isPulse1);
    void clockEnvelopes();
    void clockDMC();
    void clockLengths();
    void clockPulse();
};

extern NesAPU nesApu;