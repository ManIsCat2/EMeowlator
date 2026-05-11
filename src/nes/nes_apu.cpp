// apu base provided by aldi0123_10445 on discord, heavily rewritten by me
// audio.cpp is by me

#include "nes_apu.hpp"
#include "nes_cpu.hpp"
#include "audio.hpp"

APU apu;

const uint8_t DutyTable[4][8] = {
    {0, 1, 0, 0, 0, 0, 0, 0}, 
    {0, 1, 1, 0, 0, 0, 0, 0}, 
    {0, 1, 1, 1, 1, 0, 0, 0}, 
    {1, 0, 0, 1, 1, 1, 1, 1}  
};
const uint8_t LengthTable[32] = {
    10, 254, 20, 2, 40, 4, 80, 6, 160, 8, 60, 10, 14, 12, 26, 14,
    12, 16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30
};
const uint8_t TriTable[32] = {
    15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};
const uint16_t NoiseTimerTableNTSC[16] = {
    4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068
};
const uint16_t NoiseTimerTablePAL[16] = {
    4, 8, 14, 30, 60, 88, 118, 148, 188, 236, 354, 472, 708, 944, 1890, 3778
};
const uint16_t DMCRateTableNTSC[16] = {
    428, 380, 340, 320, 286, 254, 226, 214, 190, 160, 142, 128, 106, 84, 72, 54
};
const uint16_t DMCRateTablePAL[16] = {
    398, 354, 316, 298,
    276, 236, 210, 198,
    176, 148, 132, 118,
    98, 78, 66, 50
};

APU::APU() {}
APU::~APU() {}

void APU::reset() {
    pulse1 = PulseChannel();
    pulse2 = PulseChannel();
    triangle = TriangleChannel();
    noise = NoiseChannel();
    dmc = DMCChannel(); 
    clockCounter = 0;
    frameCounter = 0; 
    frameMode = 0;
    IRQInhibit = false;
    IRQPending = false;
    DMCIrqPending = false;
    DMCIrqEnable = false;
    frameCounterResetDelay = 0;
    delayedFrameMode = 0;
    noise.shiftRegister = 1;
}

void APU::write(uint16_t addr, uint8_t data) {
    switch (addr) {
        case 0x4000: pulse1.duty = (data & 0xC0) >> 6; pulse1.lengthHalt = (data & 0x20); pulse1.constantVolume = (data & 0x10); pulse1.volume = (data & 0x0F); break;
        case 0x4001:
            pulse1.sweepEnable = (data & 0x80) != 0;
            pulse1.sweepPeriod = (data >> 4) & 0x07;
            pulse1.sweepNegate = (data & 0x08) != 0;
            pulse1.sweepShift  = data & 0x07;
            pulse1.sweepReload = true;
            break;
        case 0x4002: pulse1.timerReload = (pulse1.timerReload & 0xFF00) | data; break;
        case 0x4003: pulse1.timerReload = (pulse1.timerReload & 0x00FF) | ((data & 0x07) << 8); pulse1.timer = pulse1.timerReload; if (pulse1.enable) pulse1.lengthCounter = LengthTable[(data & 0xF8) >> 3]; pulse1.dutySeq = 0; pulse1.envStart = true; break;

        case 0x4004: pulse2.duty = (data & 0xC0) >> 6; pulse2.lengthHalt = (data & 0x20); pulse2.constantVolume = (data & 0x10); pulse2.volume = (data & 0x0F); break;
        case 0x4005:
            pulse2.sweepEnable = (data & 0x80) != 0;
            pulse2.sweepPeriod = (data >> 4) & 0x07;
            pulse2.sweepNegate = (data & 0x08) != 0;
            pulse2.sweepShift  = data & 0x07;
            pulse2.sweepReload = true;
            break;
        case 0x4006: pulse2.timerReload = (pulse2.timerReload & 0xFF00) | data; break;
        case 0x4007: pulse2.timerReload = (pulse2.timerReload & 0x00FF) | ((data & 0x07) << 8); pulse2.timer = pulse2.timerReload; if (pulse2.enable) pulse2.lengthCounter = LengthTable[(data & 0xF8) >> 3]; pulse2.dutySeq = 0; pulse2.envStart = true; break;

        case 0x4008: triangle.lengthHalt = (data & 0x80); triangle.linearReload = data & 0x7F; break;
        case 0x400A: triangle.timerReload = (triangle.timerReload & 0xFF00) | data; break;
        case 0x400B: triangle.timerReload = (triangle.timerReload & 0x00FF) | ((data & 0x07) << 8); triangle.timer = triangle.timerReload; if (triangle.enable) triangle.lengthCounter = LengthTable[(data & 0xF8) >> 3]; triangle.linearReloadFlag = true; break;

        case 0x400C: noise.lengthHalt = (data & 0x20); noise.constantVolume = (data & 0x10); noise.volume = (data & 0x0F); break;
        case 0x400E: {
            noise.mode = (data & 0x80);
            uint16_t *noiseTimers = (uint16_t*)(globalROM.Region == ConsoleRegion::NTSC ? NoiseTimerTableNTSC : NoiseTimerTablePAL);
            noise.timerReload = noiseTimers[data & 0x0F];
            break;
        }
        case 0x400F: if (noise.enable) noise.lengthCounter = LengthTable[(data & 0xF8) >> 3]; noise.envStart = true; break;

        case 0x4010: {
            DMCIrqEnable = (data & 0x80) != 0;
            dmc.loop = (data & 0x40) != 0;
            uint16_t *dmcRates = (uint16_t*)(globalROM.Region == ConsoleRegion::NTSC ? DMCRateTableNTSC : DMCRateTablePAL);
            dmc.timerReload = dmcRates[data & 0x0F];

            if (!DMCIrqEnable)
                DMCIrqPending = false;
            break;
        }

        case 0x4011:
            dmc.outputLevel = data & 0x7F;
            break;

        case 0x4012:
            dmc.sampleAddress = 0xC000 + (data * 64);
            break;

        case 0x4013:
            dmc.sampleLength = (data * 16) + 1;
            break;

        case 0x4015:
            pulse1.enable = data & 0x01;
            if (!pulse1.enable) pulse1.lengthCounter = 0;

            pulse2.enable = data & 0x02;
            if (!pulse2.enable) pulse2.lengthCounter = 0;

            triangle.enable = data & 0x04;
            if (!triangle.enable) triangle.lengthCounter = 0;

            noise.enable = data & 0x08;
            if (!noise.enable) noise.lengthCounter = 0;

            if ((data & 0x10) == 0) {
                dmc.enable = false;
                dmc.currentLength = 0;
            } else {
                dmc.enable = true;

                if (dmc.currentLength == 0) {
                    dmc.currentAddress = dmc.sampleAddress;
                    dmc.currentLength = dmc.sampleLength;
                    dmc.sampleBufferEmpty = true;
                    dmc.bitsRemaining = 8;
                }
            }

            DMCIrqPending = false;
            break;
            
        case 0x4017:
            delayedFrameMode = (data & 0x80) >> 7;
            IRQInhibit = (data & 0x40) >> 6;
            
            IRQPending = false; 
            
            frameCounterResetDelay = (clockCounter & 1) ? 4 : 3;
            break;
    }
}

uint8_t APU::read(uint16_t addr) {
    uint8_t data = 0; 
    if (addr == 0x4015) {
        data = (cpu.dataBus & 0x20); 
        if (pulse1.lengthCounter > 0) data |= 0x01;
        if (pulse2.lengthCounter > 0) data |= 0x02;
        if (triangle.lengthCounter > 0) data |= 0x04;
        if (noise.lengthCounter > 0) data |= 0x08;
        
        if (dmc.currentLength > 0) data |= 0x10;
        
        if (IRQPending) data |= 0x40; 
        if (DMCIrqPending) data |= 0x80; 
        
        IRQPending = false; 
    }
    return data;
}

bool APU::pulseSweepMuted(PulseChannel &p, bool isPulse1) {
    if (p.timerReload < 8)
        return true;

    uint16_t change = p.timerReload >> p.sweepShift;
    uint16_t target = p.timerReload;

    if (p.sweepNegate) {
        target -= change;
        if (isPulse1) target--;
    } else {
        target += change;
    }

    return target > 0x7FF;
}

void APU::clockSweep(PulseChannel &p, bool isPulse1) {
    if (p.sweepReload) {
        p.sweepReload = false;
        p.sweepDivider = p.sweepPeriod == 0 ? 8 : p.sweepPeriod;
        return;
    }

    if (p.sweepDivider > 0) {
        p.sweepDivider--;
        return;
    }

    p.sweepDivider = p.sweepPeriod == 0 ? 8 : p.sweepPeriod;

    if (!p.sweepEnable || p.sweepShift == 0 || p.timerReload < 8)
        return;

    uint16_t change = p.timerReload >> p.sweepShift;
    uint16_t target = p.timerReload;

    if (p.sweepNegate) {
        target -= change;
        if (isPulse1) target--;
    } else {
        target += change;
    }

    if (target > 0x7FF) {
        return;
    }

    p.timerReload = target;
}

void APU::clockDMC() {
    if (dmc.sampleBufferEmpty && dmc.currentLength > 0) {
        dmc.sampleBuffer = cpu.read(dmc.currentAddress);
        dmc.sampleBufferEmpty = false;

        dmc.currentAddress++;
        if (dmc.currentAddress == 0)
            dmc.currentAddress = 0x8000;

        dmc.currentLength--;

        if (dmc.currentLength == 0) {
            if (dmc.loop) {
                dmc.currentAddress = dmc.sampleAddress;
                dmc.currentLength = dmc.sampleLength;
            } else if (DMCIrqEnable) {
                DMCIrqPending = true;
            }
        }
    }

    if (dmc.timer > 0) {
        dmc.timer--;
        return;
    }

    dmc.timer = dmc.timerReload;

    if (!dmc.silence) {
        if (dmc.shiftRegister & 1) {
            if (dmc.outputLevel <= 125)
                dmc.outputLevel += 2;
        } else {
            if (dmc.outputLevel >= 2)
                dmc.outputLevel -= 2;
        }
    }

    dmc.shiftRegister >>= 1;
    dmc.bitsRemaining--;

    if (dmc.bitsRemaining == 0) {
        dmc.bitsRemaining = 8;

        if (dmc.sampleBufferEmpty) {
            dmc.silence = true;
        } else {
            dmc.silence = false;
            dmc.shiftRegister = dmc.sampleBuffer;
            dmc.sampleBufferEmpty = true;
        }
    }
}

void APU::clockEnvelopes() {
    auto clockEnv = [](auto &c) {
        if (c.envStart) { c.envStart = false; c.envVol = 15; c.envDivider = c.volume; } 
        else {
            if (c.envDivider > 0) c.envDivider--;
            else {
                c.envDivider = c.volume;
                if (c.envVol > 0) c.envVol--;
                else if (c.lengthHalt) c.envVol = 15; 
            }
        }
    };
    clockEnv(pulse1);
    clockEnv(pulse2);
    clockEnv(noise);

    if (triangle.linearReloadFlag) triangle.linearCounter = triangle.linearReload;
    else if (triangle.linearCounter > 0) triangle.linearCounter--;
    if (!triangle.lengthHalt) triangle.linearReloadFlag = false;
}

void APU::clockLengths() {
    if (pulse1.lengthCounter > 0 && !pulse1.lengthHalt) pulse1.lengthCounter--;
    if (pulse2.lengthCounter > 0 && !pulse2.lengthHalt) pulse2.lengthCounter--;
    if (triangle.lengthCounter > 0 && !triangle.lengthHalt) triangle.lengthCounter--;
    if (noise.lengthCounter > 0 && !noise.lengthHalt) noise.lengthCounter--;
}

void APU::clockPulse() {
    clockLengths();
    clockSweep(pulse1, true);
    clockSweep(pulse2, false);
}

void APU::step() {
    if (frameCounterResetDelay > 0) {
        frameCounterResetDelay--;
        if (frameCounterResetDelay == 0) {
            frameCounter = 0;
            frameMode = delayedFrameMode;
            if (frameMode == 1) { 
                clockPulse();
                clockEnvelopes();
            }
        }
    }

    if (triangle.timer > 0) {
        triangle.timer--;
    } else {
        triangle.timer = triangle.timerReload;
        if (triangle.linearCounter > 0 && triangle.lengthCounter > 0 && triangle.timerReload >= 2) {
            triangle.dutySeq = (triangle.dutySeq + 1) % 32;
        }
    }

    if (clockCounter % 2 == 0) {
        if (pulse1.timer == 0) {
            pulse1.timer = pulse1.timerReload;
            pulse1.dutySeq = (pulse1.dutySeq + 1) & 7;
        } else {
            pulse1.timer--;
        }
        if (pulse2.timer == 0) {
            pulse2.timer = pulse2.timerReload;
            pulse2.dutySeq = (pulse2.dutySeq + 1) & 7;
        } else {
            pulse2.timer--;
        }
        if (noise.timer > 0) noise.timer--; else {
            noise.timer = noise.timerReload;
            uint16_t tap = noise.mode ? 6 : 1;
            uint16_t feedback = (noise.shiftRegister & 1) ^ ((noise.shiftRegister >> tap) & 1);

            noise.shiftRegister >>= 1;
            noise.shiftRegister |= (feedback << 14);
        }
    }
    clockDMC();

    frameCounter++;
    if (globalROM.Region == ConsoleRegion::NTSC) {
        if (frameMode == 0) { 
            if (frameCounter == 7457)  { clockEnvelopes(); }
            if (frameCounter == 14913) { clockEnvelopes(); clockPulse(); }
            if (frameCounter == 22371) { clockEnvelopes(); }

            if (frameCounter == 29828) {
                if (!IRQInhibit) IRQPending = true;
            }

            if (frameCounter == 29829) {
                if (!IRQInhibit) IRQPending = true;
                clockEnvelopes();
                clockPulse();
            }

            if (frameCounter == 29830) {
                if (!IRQInhibit) IRQPending = true;
                frameCounter = 0;
            }
        } else { 
            if (frameCounter == 7457)  {
                clockEnvelopes();
            }

            if (frameCounter == 14913) {
                clockEnvelopes();
                clockPulse();
            }

            if (frameCounter == 22371) {
                clockEnvelopes();
            }

            if (frameCounter == 37281) {
                clockEnvelopes();
                clockPulse();
            }

            if (frameCounter == 37282) { 
                frameCounter = 0;
            }
        }
    } else {
        if (frameMode == 0) {
            if (frameCounter == 8313)  { clockEnvelopes(); }
            if (frameCounter == 16627) { clockEnvelopes(); clockPulse(); }
            if (frameCounter == 24939) { clockEnvelopes(); }

            if (frameCounter == 33252) {
                if (!IRQInhibit) IRQPending = true;
            }

            if (frameCounter == 33253) {
                if (!IRQInhibit) IRQPending = true;

                clockEnvelopes();
                clockPulse();
            }

            if (frameCounter == 33254) {
                if (!IRQInhibit) IRQPending = true;

                frameCounter = 0;
            }
        } else {
            if (frameCounter == 8313)  {
                clockEnvelopes();
            }

            if (frameCounter == 16627) {
                clockEnvelopes();
                clockPulse();
            }

            if (frameCounter == 24939) {
                clockEnvelopes();
            }

            if (frameCounter == 41565) {
                clockEnvelopes();
                clockPulse();
            }

            if (frameCounter == 41566) {
                frameCounter = 0;
            }
        }
    }

    clockCounter++;
    audioSystem.advance();
}

double APU::getOutputSample() {
    double p1 = 0.0;
    double p2 = 0.0;
    double t  = 0.0;
    double n  = 0.0;
    double d  = 0.0;

    if (pulse1.enable && pulse1.lengthCounter > 0 && !pulseSweepMuted(pulse1, true)) {
        p1 = DutyTable[pulse1.duty][pulse1.dutySeq] ? (pulse1.constantVolume ? pulse1.volume : pulse1.envVol) : 0;
    }

    if (pulse2.enable && pulse2.lengthCounter > 0 && !pulseSweepMuted(pulse2, false)) {
        p2 = DutyTable[pulse2.duty][pulse2.dutySeq] ? (pulse2.constantVolume ? pulse2.volume : pulse2.envVol) : 0;
    }

    if (triangle.enable && triangle.lengthCounter > 0 && triangle.linearCounter > 0) {
        t = TriTable[triangle.dutySeq];
    }

    if (noise.enable && noise.lengthCounter > 0 && (noise.shiftRegister & 0x0001) == 0) {
        n = noise.constantVolume ? noise.volume : noise.envVol;
    }

    if (dmc.enable) {
        d = dmc.outputLevel;
    }

    p1 *= (pulse1Volume   / 50.0) * (masterVolume / 50.0);
    p2 *= (pulse2Volume   / 50.0) * (masterVolume / 50.0);
    t  *= (triangleVolume / 50.0) * (masterVolume / 50.0);
    n  *= (noiseVolume    / 50.0) * (masterVolume / 50.0);
    d  *= (dmcVolume      / 50.0) * (masterVolume / 50.0);

    double pulseOut = 0.0;
    double pulseSum = p1 + p2;
    if (pulseSum > 0.0) {
        pulseOut = 95.52 / ((8128.0 / pulseSum) + 100.0);
    }

    double tndIndex = (3.0 * t) + (2.0 * n) + d;
    double tndOut = 0.0;
    if (tndIndex > 0.0) {
        tndOut = 163.67 / ((24329.0 / tndIndex) + 100.0);
    }

    return pulseOut + tndOut;
}