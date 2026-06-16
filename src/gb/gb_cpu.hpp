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

    bool paused = false;

    uint16_t divCounter = 0;
    uint16_t timerCounter = 0;
    uint8_t DIV = 0;
    uint8_t TIMA = 0;
    uint8_t TMA = 0;
    uint8_t TAC = 0;
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
    void updateTimers(uint8_t cycles);

    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t value);
    
    void push(uint8_t value);
    uint8_t pop();


    uint8_t IF = 0xE1;
    uint8_t IE = 0x00;
    uint8_t joypadReg = 0x30;
private:
    uint64_t cycles = 0;
    bool halted = false;

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

    uint8_t opINC(uint8_t value) {
        uint8_t result = value + 1;
        
        F &= FlagC; 
        if (result == 0) F |= FlagZ;
        if ((value & 0x0F) == 0x0F) F |= FlagH;
        
        return result;
    }
    uint8_t opSUB(uint8_t val) {
        F = FlagN;
        if ((A & 0x0F) < (val & 0x0F)) {
            F |= FlagH;
        }
        if (A < val) {
            F |= FlagC;
        }
        uint8_t result = A - val;
        if (result == 0) {
            F |= FlagZ;
        }
        return result;
    }
    uint8_t opSBC(uint8_t val) {
        uint8_t carry = (F & FlagC) ? 1 : 0;
        int result = A - val - carry;
        
        F = FlagN;
        if (((A & 0x0F) - (val & 0x0F) - carry) < 0) {
            F |= FlagH;
        }
        if (result < 0) {
            F |= FlagC;
        }
        
        uint8_t res8 = (uint8_t)result;
        if (res8 == 0) {
            F |= FlagZ;
        }
        return res8;
    }

    uint8_t opADD(uint8_t val) {
        uint16_t result = A + val;
        F = 0;
        if ((uint8_t)result == 0) {
            F |= FlagZ;
        }
        if (((A & 0x0F) + (val & 0x0F)) > 0x0F) {
            F |= FlagH;
        }
        if (result > 0xFF) {
            F |= FlagC;
        }
        return (uint8_t)result;
    }
    uint16_t opADD16(uint16_t val) {
        uint32_t hl = getHL();
        uint32_t result = hl + val;
        
        F &= ~(FlagN | FlagH | FlagC);
        if (((hl & 0x0FFF) + (val & 0x0FFF)) > 0x0FFF) {
            F |= FlagH;
        }
        if (result > 0xFFFF) {
            F |= FlagC;
        }
        
        return (uint16_t)result;
    }

    uint8_t opADC(uint8_t val) {
        uint8_t carry = (F & FlagC) ? 1 : 0;
        int result = A + val + carry;
        F = 0;
        if (((A & 0x0F) + (val & 0x0F) + carry) > 0x0F) {
            F |= FlagH;
        }
        if (result > 0xFF) {
            F |= FlagC;
        }
        uint8_t res8 = (uint8_t)result;
        if (res8 == 0) {
            F |= FlagZ;
        }
        return res8;
    }

    uint8_t opOR(uint8_t val) {
        A |= val;
        F = (A == 0) ? FlagZ : 0;
        return A;
    }

    uint8_t opXOR(uint8_t value) {
        uint8_t result = A ^ value;

        F = 0;
        if (result == 0) {
            F |= FlagZ;
        }

        return result;
    }

    uint8_t opAND(uint8_t value) {
        uint8_t result = A & value;

        F = FlagH;
        if (result == 0) {
            F |= FlagZ;
        }

        return result;
    }

    uint8_t cbRL(uint8_t v) {
        uint8_t oldCarry = (F & FlagC) ? 1 : 0;
        uint8_t newCarry = (v & 0x80) ? 1 : 0;

        v = (v << 1) | oldCarry;

        F = 0;
        if (v == 0) F |= FlagZ;
        if (newCarry) F |= FlagC;

        return v;
    }

    uint8_t cbRR(uint8_t v) {
        uint8_t oldCarry = (F & FlagC) ? 1 : 0;
        uint8_t newCarry = (v & 0x01) ? 1 : 0;

        v = (v >> 1) | (oldCarry << 7);

        F = 0;
        if (v == 0) F |= FlagZ;
        if (newCarry) F |= FlagC;

        return v;
    }

    uint8_t cbSLA(uint8_t v) {
        uint8_t carry = v & 0x80;

        v <<= 1;

        F = 0;
        if (v == 0) F |= FlagZ;
        if (carry) F |= FlagC;

        return v;
    }

    uint8_t cbSRL(uint8_t v) {
        uint8_t carry = v & 1;

        v >>= 1;

        F = 0;
        if (v == 0) F |= FlagZ;
        if (carry) F |= FlagC;

        return v;
    }

    uint8_t cbSWAP(uint8_t v) {
        v = (v << 4) | (v >> 4);

        F = (v == 0) ? FlagZ : 0;

        return v;
    }

    void cbBITHalf(int bit, uint8_t value) {
        F &= FlagC;
        F |= FlagH;
        F &= ~FlagN;

        if (!(value & (1 << bit)))
            F |= FlagZ;
        else
            F &= ~FlagZ;
    }

    void cbBIT(uint8_t opcode) {
        int bit = (opcode >> 3) & 7;
        int reg = opcode & 7;

        switch (reg) {
            case 0: cbBITHalf(bit, B); break;
            case 1: cbBITHalf(bit, C); break;
            case 2: cbBITHalf(bit, D); break;
            case 3: cbBITHalf(bit, E); break;
            case 4: cbBITHalf(bit, H); break;
            case 5: cbBITHalf(bit, L); break;
            case 6: cbBITHalf(bit, read(getHL())); break;
            case 7: cbBITHalf(bit, A); break;
        }
    }

    uint8_t cbSETValue(int bit, uint8_t value) {
        return value | (1 << bit);
    }

    void cbSET(uint8_t opcode) {
        int bit = (opcode >> 3) & 7;
        int reg = opcode & 7;

        uint16_t hl = getHL();

        switch (reg) {
            case 0: B = cbSETValue(bit, B); break;
            case 1: C = cbSETValue(bit, C); break;
            case 2: D = cbSETValue(bit, D); break;
            case 3: E = cbSETValue(bit, E); break;
            case 4: H = cbSETValue(bit, H); break;
            case 5: L = cbSETValue(bit, L); break;

            case 6: {
                uint8_t v = read(hl);
                v = cbSETValue(bit, v);
                write(hl, v);
                break;
            }

            case 7: A = cbSETValue(bit, A); break;
        }
    }

    uint8_t cbRESValue(int bit, uint8_t value) {
        return value & ~(1 << bit);
    }

    void cbRES(uint8_t opcode) {
        int bit = (opcode >> 3) & 7;
        int reg = opcode & 7;

        uint16_t hl = getHL();

        switch (reg) {
            case 0: B = cbRESValue(bit, B); break;
            case 1: C = cbRESValue(bit, C); break;
            case 2: D = cbRESValue(bit, D); break;
            case 3: E = cbRESValue(bit, E); break;
            case 4: H = cbRESValue(bit, H); break;
            case 5: L = cbRESValue(bit, L); break;

            case 6: {
                uint8_t v = read(hl);
                v = cbRESValue(bit, v);
                write(hl, v);
                break;
            }

            case 7: A = cbRESValue(bit, A); break;
        }
    }

    uint8_t fetch() { return dataBus = read(PC++); }
    uint16_t fetch16() { return fetch() | (fetch() << 8); }
};

extern GbCPU gbCpu;