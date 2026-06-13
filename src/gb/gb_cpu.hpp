#pragma once

#include <cstdint>
#include <iostream>
#include <array>
#include <stdio.h>
#include "gb_console.hpp"
#include "gb_bus.hpp"
#include "../main.hpp"

class GbCPU : public HasGBBus {
public:
    GbCPU() { }

    enum Flags {
        FlagC = (1 << 4),
        FlagH = (1 << 5),
        FlagN = (1 << 6),
        FlagZ = (1 << 7),
    };

    bool CPUPaused = false;
    
    uint8_t WRAM[8192];
    uint8_t HRAM[128];
    
    uint8_t dataBus = 0;
    bool IME = false;

    void reset();

    void GetInfo(uint8_t *AReg, uint8_t *FReg, uint8_t *BReg, uint8_t *CReg, 
                 uint8_t *DReg, uint8_t *EReg, uint8_t *HReg, uint8_t *LReg, 
                 uint16_t *PCReg, uint16_t *SPReg) {
        *AReg = A; *FReg = F;
        *BReg = B; *CReg = C;
        *DReg = D; *EReg = E;
        *HReg = H; *LReg = L;
        *PCReg = PC;
        *SPReg = SP;
    }

    void SetInfo(uint8_t AReg, uint8_t FReg, uint8_t BReg, uint8_t CReg, 
                 uint8_t DReg, uint8_t EReg, uint8_t HReg, uint8_t LReg, 
                 uint16_t PCReg, uint16_t SPReg) {
        A = AReg; F = FReg & 0xF0;
        B = BReg; C = CReg;
        D = DReg; E = EReg;
        H = HReg; L = LReg;
        PC = PCReg;
        SP = SPReg;
    }

    void run(uint32_t maxCycles);
    void execute(uint8_t opcode);
    void executeCB(uint8_t opcode);
    void handleInterrupts();

    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t value);
    
    void push(uint8_t value);
    uint8_t pop();


    uint8_t IF = 0xE1;
    uint8_t IE = 0x00;
    uint8_t joypadReg = 0x30;
private:
    uint64_t cycles = 0;
    
    uint8_t A = 0, F = 0;
    uint8_t B = 0, C = 0;
    uint8_t D = 0, E = 0;
    uint8_t H = 0, L = 0;

    uint16_t PC = 0;
    uint16_t SP = 0;

    uint16_t getAF() { return (A << 8) | F; }
    uint16_t getBC() { return (B << 8) | C; }
    uint16_t getDE() { return (D << 8) | E; }
    uint16_t getHL() { return (H << 8) | L; }

    void setAF(uint16_t val) { A = (val >> 8); F = val & 0xF0; }
    void setBC(uint16_t val) { B = (val >> 8); C = val & 0xFF; }
    void setDE(uint16_t val) { D = (val >> 8); E = val & 0xFF; }
    void setHL(uint16_t val) { H = (val >> 8); L = val & 0xFF; }

    uint8_t opINC(uint8_t value);

    uint8_t fetch() { return dataBus = read(PC++); }
    uint16_t fetch16() { return fetch() | (fetch() << 8); }
};

extern GbCPU gbCpu;