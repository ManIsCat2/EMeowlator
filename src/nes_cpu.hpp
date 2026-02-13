#pragma once

#include <cstdint>
#include <iostream>
#include <array>
#include "nes_ppu.hpp"
#include "main.hpp"

#include <stdio.h>

class CPU {
public:
    CPU() { reset(); }

    bool CPUPaused = false;
    uint8_t RAM[RAM_SIZE];
    uint8_t PrgRAM[0x2000];

    void reset() {
        A = X = Y = 0;
        SP = 0xFD;
        P = 0x24;
        PC = read16(0xFFFC);
        cycles = 0;
    }

    bool NMIDetector = false;
    bool doNMI = false;
    bool doIRQ = false;

    void GetInfo(uint8_t *AReg, uint8_t *XReg, uint8_t *YReg, uint16_t *PCPtr, uint8_t *SPPtr, uint8_t *PPtr) {
        *AReg = A;
        *XReg = X;
        *YReg = Y;
        *PCPtr = PC;
        *SPPtr = SP;
        *PPtr = P;
    }

    void SetInfo(uint8_t AReg, uint8_t XReg, uint8_t YReg, uint16_t PCNew, uint8_t SPNew, uint8_t PNew) {
        A = AReg;
        X = XReg;
        Y = YReg;
        PC = PCNew;
        SP = SPNew;
        P = PNew;
    }

    void run(uint32_t maxCycles);

    void execute(uint8_t opcode);
    void SetZN(uint8_t value);
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t value);
    uint16_t read16(uint16_t addr);
    void push(uint8_t value);
    uint8_t pop();

private:
    uint64_t cycles;
    uint8_t A, X, Y;
    uint16_t PC;
    uint8_t SP;
    uint8_t P;

    uint8_t fetch() { return read(PC++); }

    uint16_t fetch16() { return fetch() | (fetch() << 8); }
};

extern CPU cpu;