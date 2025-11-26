#include "emulator.hpp"

//cpu
ushort PC = 0;
uint8_t A = 0;
uint8_t X = 0;
uint8_t Y = 0;
bool FlagCarry = false;
bool FlagZero = false;
bool FlagInterruptDisable = false;
bool FlagDecimal = false;
bool FlagOverflow = false;
bool FlagNegative = false;
uint8_t StackPtr = 0;

//ppu
uint8_t VRAM[0x800] = {0};
uint8_t PaletteRAM[0x20] = {0};
bool WriteLatch = false;
ushort TransferAddress = 0;
ushort VRAMAddress = 0;
ushort TempVRAMAddress = 0;
bool VRAMInc32Mode = false;
uint8_t ReadBuffer = 0;
int Dot = 0;
int Scanline = 0;
bool VBlank = false;
bool Mask8pxMaskBG = false;
bool Mask8pxMaskSprites = false;
bool MaskRenderBG = false;
bool MaskRenderSprites = false;
int NametableSelect = 0;
bool SpritePatternTable = false;
bool BGPatternTable = false;
bool Use8x16Sprites = false;
bool EnableNMI = false;
ushort ShiftRegPatternL = 0;
ushort ShiftRegPatternH = 0;
ushort ShiftRegAttrL = 0;
ushort ShiftRegAttrH = 0;
ushort EightStepPatternLowBitPlane = 0;
ushort EightStepPatternHighBitPlane = 0;
uint8_t EightStepAttr = 0;
ushort AddrBus = 0;
uint8_t EightStepTemp = 0;
uint8_t EightStepNextChar = 0;
uint8_t ScrollFineX = 0;

//mem
uint8_t RAM[0x800] = {0};
uint8_t ROM[0xA000] = {0};
uint8_t Header[0x10] = {0};
uint8_t CHRData[0x2000] = {0};

bool CPUHalted = false;

uint8_t ReadPPU(ushort Address) {
    if (Address < 0x2000) {
        return CHRData[Address];
    } else if (Address < 0x3F00) {
        if ((Header[6] & 1) == 0) {
            //horizontal
            return VRAM[(Address & 0x3FF) | (Address & 0x800) >> 1];
        } else {
            //vertical
            return VRAM[Address & 0x7FF];
        }
    } else {
        if ((Address & 3) == 0) {
            return PaletteRAM[Address & 0x0F];
        } else {
            return PaletteRAM[Address & 0x1F];
        }
    }
}


uint8_t Read(ushort Address) {
    if (Address < 0x800) {
        return RAM[Address];
    } else if (Address < 0x4000) {
        Address &= 0x2007;
        switch (Address) {
            case 0x2002: {
                uint8_t Status = 0;
                Status |= (VBlank ? 0x80 : 0);
                Status |= 0x40;
                VBlank = false;
                WriteLatch = false;
                return Status;
            }
            case 0x2007: {
                uint8_t Temp = ReadBuffer;
                if (VRAMAddress > 0x3F00) {
                    Temp = ReadPPU(VRAMAddress);
                } else {
                    ReadBuffer = ReadPPU(VRAMAddress);
                }
                VRAMAddress += VRAMInc32Mode ? 32 : 1;
                VRAMAddress &= 0x3FFF;
                return Temp;
            }
            default:
                return 0;
        }
    } else if (Address >= 0x8000) {
        return ROM[Address-0x8000];
    }
    return 0;
}

void Write(ushort Address, uint8_t Value) {
    if (Address < 0x2000) {
        RAM[Address & 0x7FF] = Value;
    } else if (Address < 0x4000) {
        Address &= 0x2007;
        switch (Address) {
            case 0x2000: // PPUCTRL
                NametableSelect = Value & 0x03;
                VRAMInc32Mode = (Value & 0x04) != 0;
                SpritePatternTable = (Value & 0x08) != 0;
                BGPatternTable = (Value & 0x10) != 0;
                Use8x16Sprites = (Value & 0x20) != 0;
                EnableNMI = (Value & 0x80) != 0;
                break;
            case 0x2001: // PPUMASK
                Mask8pxMaskBG = (Value & 0x02) != 0;
                Mask8pxMaskSprites = (Value & 0x04) != 0;
                MaskRenderBG = (Value & 0x08) != 0;
                MaskRenderSprites = (Value & 0x10) != 0;
                break;
            case 0x2002: // PPUSTATUS
                break;
            case 0x2003: // OAMADDR
                break;
            case 0x2004: // OAMDATA
                break;
            case 0x2005: // PPUSCROLL
                if (!WriteLatch) {
                    ScrollFineX = Value & 7;
                    TempVRAMAddress = (ushort)((TempVRAMAddress & 0b0111111111100000) | (Value >> 3));
                } else {
                    TransferAddress = (ushort)((TempVRAMAddress & 0b0000110000011111) | (((Value & 0xF8) << 2) | ((Value & 7) << 12)));
                }
                WriteLatch = !WriteLatch;
                break;
            case 0x2006: // PPUADDR
                if (!WriteLatch) {
                    TempVRAMAddress = ((Value & 0x3F) << 8);
                } else {
                    VRAMAddress = (ushort)(TempVRAMAddress | Value);
                    TransferAddress = VRAMAddress;
                }
                WriteLatch = !WriteLatch;
                break;
            case 0x2007: // PPUDATA
                if (VRAMAddress < 0x2000) {
                    if (Header[5] == 0) {
                        CHRData[VRAMAddress] = Value;
                    }
                } else if (VRAMAddress < 0x3F00) {
                    if ((Header[6] & 1) == 0) {
                        //horizontal
                        VRAM[(VRAMAddress & 0x3FF) | (VRAMAddress & 0x800) >> 1] = Value;
                    } else {
                        //vert
                        VRAM[VRAMAddress & 0x7FF] = Value;
                    }
                } else {
                    if ((VRAMAddress & 3) == 0) {
                        PaletteRAM[VRAMAddress & 0x0F] = Value;
                    } else {
                        PaletteRAM[VRAMAddress & 0x1F] = Value;
                    }
                }
                VRAMAddress += VRAMInc32Mode ? 32 : 1;
                VRAMAddress &= 0x3FFF;
                break;
        }
    }
}

void Reset(const char* RomPath) {
    FILE *RomFile = fopen(RomPath, "rb");
    if (!RomFile) {
        printf("Can't open \"%s\"\n", RomPath);
        exit(1);
    }
    fseek(RomFile, 0, SEEK_END);
    size_t RomSize = ftell(RomFile);

    fseek(RomFile, 0, SEEK_SET);
    fread(Header, 1, 0x10, RomFile);

    fseek(RomFile, 0x10, SEEK_SET);
    fread(ROM, 1, RomSize - 0x10, RomFile);

    fseek(RomFile, 0x8010, SEEK_SET);
    fread(CHRData, 1, RomSize - 0x8010, RomFile);

    uint8_t PCL = Read(0xFFFC);
    uint8_t PCH = Read(0xFFFD);
    PC = (ushort)((PCH * 0x100) + PCL);
    printf("PC at 0x%x\n", PC);

    FlagInterruptDisable = true;
    StackPtr = 0xFD;
}

void SetZN(uint8_t &Val) {
    FlagZero = Val == 0;
    FlagNegative = Val > 127;
}

uint8_t BranchIf(bool Cond) {
    int8_t Offset = (int8_t)Read(PC++);
    if (Cond) {
        PC += Offset;
        return 3;
    } else {
        return 2;
    }
}

void LoadImm(uint8_t &Reg) {
    Reg = Read(PC++);
    SetZN(Reg);
}

void LoadAbs(uint8_t &Reg) {
    uint8_t AddrLow = Read(PC++);
    uint8_t AddrHigh = Read(PC++);
    uint16_t Addr = (AddrHigh*0x100) + AddrLow;
    Reg = Read(Addr);
    SetZN(Reg);
}

void LoadZP(uint8_t &Reg) {
    uint8_t Addr = Read(PC++);
    Reg = Read(Addr);
    SetZN(Reg);
}

void StoreAbs(uint8_t &Reg) {
    uint8_t AddrLow = Read(PC++);
    uint8_t AddrHigh = Read(PC++);
    Write(AddrHigh*0x100+AddrLow, Reg);
}

void StoreZP(uint8_t &Reg) {
    uint8_t Addr = Read(PC++);
    Write(Addr, Reg);
}

void Push(uint8_t Value) {
    Write((ushort)(0x100+StackPtr), Value);
    StackPtr--;
}

uint8_t Pull() {
    StackPtr++;
    return Read((ushort)(0x100+StackPtr));
}

void OpASL(ushort Address, uint8_t &Val) {
    FlagCarry = Val > 127;
    Val <<= 1;
    Write(Address, Val);
    SetZN(Val);
}

void OpROL(ushort Addr, uint8_t &Val) {
    bool OldCarry = FlagCarry;
    FlagCarry = Val > 127;
    Val = (Val << 1) | (OldCarry ? 1 : 0);
    Write(Addr, Val);
    SetZN(Val);
}

void OpROR(ushort Addr, uint8_t &Val) {
    bool OldCarry = FlagCarry;
    FlagCarry = (Val & 0x01) != 0;
    Val = (Val >> 1) | (OldCarry ? 0x80 : 0);
    Write(Addr, Val);
    SetZN(Val);
}

void OpLSR(ushort Addr, uint8_t &Val) {
    FlagCarry = (Val & 0x01) != 0;
    Val >>= 1;
    Write(Addr, Val);
    SetZN(Val);
}

void OpINC(ushort Addr, uint8_t &Val) {
    Val++;
    Write(Addr, Val);
    SetZN(Val);
}

void OpDEC(ushort Addr, uint8_t &Val) {
    Val--;
    Write(Addr, Val);
    SetZN(Val);
}

void OpORA(uint8_t &Reg, uint8_t Val) {
    Reg |= Val; SetZN(Reg);
}
void OpAND(uint8_t &Reg, uint8_t Val) {
    Reg &= Val; SetZN(Reg);
}
void OpEOR(uint8_t &Reg, uint8_t Val) {
    Reg ^= Val; SetZN(Reg);
}

void OpADC(uint8_t &Reg, uint8_t Val) {
    uint16_t Sum = Reg + Val + (FlagCarry ? 1 : 0);
    FlagCarry = Sum > 0xFF;
    FlagOverflow = (~(Reg ^ Val) & (Reg ^ Sum) & 0x80) != 0;
    Reg = Sum & 0xFF;
    SetZN(Reg);
}

void OpSBC(uint8_t &Reg, uint8_t Val) {
    OpADC(Reg, Val ^ 0xFF);
}

void OpCMP(uint8_t Reg, uint8_t Val) {
    uint8_t Result = Reg - Val;
    FlagCarry = Reg >= Val;
    SetZN(Result);
}

void OpBIT(uint8_t Val) {
    FlagZero = (A & Val) == 0;
    FlagNegative = (Val & 0x80) != 0;
    FlagOverflow = (Val & 0x40) != 0;
}

extern void EmulatePPU(void);
bool NMILvlDetector = false;
bool DoNMI = false;

void EmulateCPU(void) {
    bool PrevLvlDetect = NMILvlDetector;
    NMILvlDetector = VBlank && EnableNMI;

    if (!PrevLvlDetect && NMILvlDetector) {
        DoNMI = true;
    }

    uint8_t Cycles = 0;
    uint8_t OpCode = 0x00;
    if (!DoNMI) {
        OpCode = Read(PC++);
    } else {
        OpCode = 0x00;
    }
    switch (OpCode) {
        case 0x02: // HLT
            CPUHalted = true;
            printf("Halt on 0x%x\n", PC);
            break;

        case 0x10: // BPL
            Cycles = BranchIf(!FlagNegative);
            break;
        case 0x30: // BMI
            Cycles = BranchIf(FlagNegative);
            break;
        case 0x50: // BVC
            Cycles = BranchIf(!FlagOverflow);
            break;
        case 0x70: // BVS
            Cycles = BranchIf(FlagOverflow);
            break;
        case 0x90: // BCC
            Cycles = BranchIf(!FlagCarry);
            break;
        case 0xB0: // BCS
            Cycles = BranchIf(FlagCarry);
            break;
        case 0xD0: // BNE
            Cycles = BranchIf(!FlagZero);
            break;
        case 0xF0: // BEQ
            Cycles = BranchIf(FlagZero);
            break;

        case 0xA0: // LDY imm
            LoadImm(Y);
            Cycles = 2;
            break;
        case 0xA2: // LDX imm
            LoadImm(X);
            Cycles = 2;
            break;
        case 0xA5: // LDA zp
            LoadZP(A);
            Cycles = 3;
            break;
        case 0xA6: // LDX zp
            LoadZP(X);
            Cycles = 3;
            break;
        case 0xA4: // LDY zp
            LoadZP(Y);
            Cycles = 3;
            break;
        case 0xAC: // LDY abs
            LoadAbs(Y);
            Cycles = 3;
            break;
        case 0xBC: { // LDY abs,X
            uint8_t AddrLow = Read(PC++);
            uint8_t AddrHigh = Read(PC++);
            uint16_t Addr = ((AddrHigh*256) + AddrLow) + X;
            Y = Read(Addr);
            SetZN(Y);
            Cycles = 4;
            break;
        }
        case 0xAE: // LDX abs
            LoadAbs(X);
            Cycles = 3;
            break;
        case 0xAD: // LDA abs
            LoadAbs(A);
            Cycles = 3;
            break;
        case 0xBD: { // LDA abs,X
            uint8_t AddrLow = Read(PC++);
            uint8_t AddrHigh = Read(PC++);
            uint16_t Addr = ((AddrHigh*256) + AddrLow) + X;
            A = Read(Addr);
            SetZN(A);
            Cycles = 4;
            break;
        }
        case 0xB9: { // LDA abs,Y
            uint8_t AddrLow = Read(PC++);
            uint8_t AddrHigh = Read(PC++);
            uint16_t Addr = ((AddrHigh*256) + AddrLow) + Y;
            A = Read(Addr);
            SetZN(A);
            Cycles = 4;
            break;
        }
        case 0xBE: { // LDX abs,Y
            uint8_t AddrLow = Read(PC++);
            uint8_t AddrHigh = Read(PC++);
            uint16_t Addr = ((AddrHigh*256)+AddrLow) + Y;
            X = Read(Addr);
            SetZN(X);
            Cycles = 4;
            break;
        }
        case 0xB5: { // LDA zp,X
            uint8_t Addr = (Read(PC++) + X);
            A = Read(Addr);
            SetZN(A);
            Cycles = 4;
            break;
        }
        case 0xA1: { // LDA (ind,X)
            uint8_t Ptr = (Read(PC++) + X);
            uint16_t Addr = Read(Ptr) | (Read((Ptr + 1)) << 8);
            A = Read(Addr);
            SetZN(A);
            Cycles = 6;
            break;
        }
        case 0xB1: { // LDA (ind),Y
            uint8_t Ptr = Read(PC++);
            uint16_t Addr = Read(Ptr) | (Read((Ptr + 1)) << 8);
            Addr += Y;
            A = Read(Addr);
            SetZN(A);
            Cycles = 5;
            break;
        }
        case 0xA9: // LDA imm
            LoadImm(A);
            Cycles = 2;
            break;
        
        case 0x84: // STY zp
            StoreZP(Y);
            Cycles = 3;
            break;
        case 0x85: // STA zp
            StoreZP(A);
            Cycles = 3;
            break;
        case 0x95: { // STA zp,X
            uint8_t Addr = (Read(PC++) + X);
            Write(Addr, A);
            Cycles = 4;
            break;
        }
        case 0x86: // STX zp
            StoreZP(X);
            Cycles = 3;
            break;
        case 0x8C: // STY abs
            StoreAbs(Y);
            Cycles = 4;
            break;
        case 0x8D: // STA abs
            StoreAbs(A);
            Cycles = 4;
            break;
        case 0x99: { // STA abs,Y
            uint8_t AddrLow = Read(PC++);
            uint8_t AddrHigh = Read(PC++);
            uint16_t Addr = (AddrHigh*256) + AddrLow + Y;
            Write(Addr, A);
            Cycles = 5;
            break;
        }
        case 0x9D: { // STA abs,X
            uint8_t AddrLow = Read(PC++);
            uint8_t AddrHigh = Read(PC++);
            uint16_t Addr = (AddrHigh*256) + AddrLow + X;
            Write(Addr, A);
            Cycles = 5;
            break;
        }
        case 0x8E: // STX abs
            StoreAbs(X);
            Cycles = 4;
            break;
        case 0x91: { // STA (ind),Y
            uint8_t Ptr = Read(PC++);
            uint16_t Addr = Read(Ptr) | (Read((Ptr + 1)) << 8);
            Addr += Y;
            Write(Addr, A);
            Cycles = 6;
            break;
        }
        case 0x81: { // STA (ind,X)
            uint8_t Ptr = (Read(PC++) + X);
            uint16_t Addr = Read(Ptr) | (Read((Ptr + 1)) << 8);
            Write(Addr, A);
            Cycles = 6;
            break;
        }

        case 0x20: { // JSR
            uint8_t AddrLow = Read(PC++);
            uint8_t AddrHigh = Read(PC);
            Push((uint8_t)(PC/256));
            Push((uint8_t)PC);
            PC = (ushort)((AddrHigh*0x100) + AddrLow);
            Cycles = 6;
            break;
        }
        case 0x60: { // RTS
            uint8_t AddrLow = Pull();
            uint8_t AddrHigh = Pull();
            PC = (ushort)((AddrHigh*0x100) + AddrLow);
            PC++;
            Cycles = 6;
            break;
        }

        case 0x48: // PHA
            Push(A);
            Cycles = 3;
            break;
        case 0x68: // PLA
            A = Pull();
            SetZN(A);
            Cycles = 4;
            break;
        case 0x08: { // PHP
            uint8_t Flags = 0;
            Flags += (FlagCarry ? 0x01 : 0);
            Flags += (FlagZero ? 0x02 : 0);
            Flags += (FlagInterruptDisable ? 0x04 : 0);
            Flags += (FlagDecimal ? 0x08 : 0);
            Flags += 0x10;
            Flags += 0x20;
            Flags += (FlagOverflow ? 0x40 : 0);
            Flags += (FlagNegative ? 0x80 : 0);
            Push(Flags);
            Cycles = 3;
            break;
        }
        case 0x28: { // PLP
            uint8_t Flags = Pull();
            FlagCarry = (Flags & 0x01) != 0;
            FlagZero = (Flags & 0x02) != 0;
            FlagInterruptDisable = (Flags & 0x04) != 0;
            FlagDecimal = (Flags & 0x08) != 0;
            FlagOverflow = (Flags & 0x40) != 0;
            FlagNegative = (Flags & 0x80) != 0;
            Cycles = 3;
            break;
        }

        case 0xE8: // INX
            X++;
            SetZN(X);
            Cycles = 2; 
            break;
        case 0xC8: // INY
            Y++;
            SetZN(Y);
            Cycles = 2; 
            break;
        case 0xCA: // DEX
            X--;
            SetZN(X);
            Cycles = 2;
            break;
        case 0x88: // DEY
            Y--;
            SetZN(Y);
            Cycles = 2;
            break;

        case 0xAA: // TAX
            X = A;
            SetZN(X);
            Cycles = 2;
            break;
        case 0x8A: // TXA
            A = X;
            SetZN(A);
            Cycles = 2;
            break;
        case 0xA8: // TAY
            Y = A;
            SetZN(Y);
            Cycles = 2;
            break;
        case 0x98: // TYA
            A = Y;
            SetZN(A);
            Cycles = 2;
            break;
        case 0x9A: // TXS
            StackPtr = X;
            Cycles = 2;
            break;
        case 0xBA: // TSX
            X = StackPtr;
            SetZN(X);
            Cycles = 2; 
            break;

        case 0x38: // SEC
            FlagCarry = true;
            Cycles = 2;
            break;
        case 0x18: // CLC
            FlagCarry = false;
            Cycles = 2;
            break;
        case 0xB8: // CLV
            FlagOverflow = false;
            Cycles = 2;
            break;
        case 0x78: // SEI
            FlagInterruptDisable = true;
            Cycles = 2;
            break;
        case 0x58: // CLI
            FlagInterruptDisable = false;
            Cycles = 2;
            break;
        case 0xF8: // SED
            FlagDecimal = true;
            Cycles = 2;
            break;
        case 0xD8: // CLD
            FlagDecimal = false;
            Cycles = 2;
            break;  
        case 0xEA: // NOP
            Cycles = 2;
            break;

        case 0x0A: // ASL A
            FlagCarry = A > 127;
            A <<= 1;
            SetZN(A);
            Cycles = 2;
            break;
        case 0x0E: { // ASL abs
            uint8_t AddrLow = Read(PC++);
            uint8_t AddrHigh = Read(PC++);
            uint16_t Addr = (AddrHigh*256) + AddrLow;
            uint8_t Val = Read(Addr);
            OpASL(Addr, Val);
            Cycles = 6;
            break;
        }
        case 0x06: { // ASL zp
            uint8_t Addr = Read(PC++);
            uint8_t Val = Read(Addr);
            OpASL(Addr, Val);
            Cycles = 5;
            break;
        }

        case 0x2A: { // ROL A
            bool OldCarry = FlagCarry;
            FlagCarry = (A & 0x80) != 0;
            A = (A << 1) | (OldCarry ? 1 : 0);
            SetZN(A);
            Cycles = 2;
            break;
        }
        case 0x26: { // ROL zp
            uint8_t Addr = Read(PC++);
            uint8_t Val = Read(Addr);
            OpROL(Addr, Val);
            Cycles = 5;
            break;
        }
        case 0x2E: { // ROL abs
            uint8_t AddrLow = Read(PC++);
            uint8_t AddrHigh = Read(PC++);
            uint16_t Addr = (AddrHigh*256) + AddrLow;
            uint8_t Val = Read(Addr);
            OpROL(Addr, Val);
            Cycles = 6;
            break;
        }

        case 0x6A: { // ROR A
            bool OldCarry = FlagCarry;
            FlagCarry = (A & 0x01) != 0;
            A = (A >> 1) | (OldCarry ? 0x80 : 0);
            SetZN(A);
            Cycles = 2;
            break;
        }
        case 0x66: { // ROR zp
            uint8_t Addr = Read(PC++);
            uint8_t Val = Read(Addr);
            OpROR(Addr, Val);
            Cycles = 5;
            break;
        }
        case 0x7E: { // ROR abs,X
            uint8_t AddrLow = Read(PC++);
            uint8_t AddrHigh = Read(PC++);
            uint16_t Addr = (AddrHigh*256) + AddrLow + X;
            uint8_t Val = Read(Addr);
            OpROR(Addr, Val);
            Cycles = 7;
            break;
        }

        case 0x4A: // LSR A
            FlagCarry = (A & 0x01) != 0;
            A >>= 1;
            SetZN(A);
            Cycles = 2;
            break;
        case 0x46: { // LSR zp
            uint8_t Addr = Read(PC++);
            uint8_t Val = Read(Addr);
            OpLSR(Addr, Val);
            Cycles = 5;
            break;
        }
        case 0x4E: { // LSR abs
            uint8_t AddrLow = Read(PC++);
            uint8_t AddrHigh = Read(PC++);
            uint16_t Addr = (AddrHigh*256) + AddrLow;
            uint8_t Val = Read(Addr);
            OpLSR(Addr, Val);
            Cycles = 6;
            break;
        }

        case 0xE6: { // INC zp
            uint8_t Addr = Read(PC++);
            uint8_t Val = Read(Addr);
            OpINC(Addr, Val);
            Cycles = 5;
            break;
        }
        case 0xEE: { // INC abs
            uint8_t AddrLow = Read(PC++);
            uint8_t AddrHigh = Read(PC++);
            uint16_t Addr = (AddrHigh << 8) | AddrLow;
            uint8_t Val = Read(Addr);
            OpINC(Addr, Val);
            Cycles = 6;
            break;
        }

        case 0xC6: { // DEC zp
            uint8_t Addr = Read(PC++);
            uint8_t Val = Read(Addr);
            OpDEC(Addr, Val);
            Cycles = 5;
            break;
        }
        case 0xCE: { // DEC abs
            uint8_t AddrLow = Read(PC++);
            uint8_t AddrHigh = Read(PC++);
            uint16_t Addr = (AddrHigh*256) + AddrLow;
            uint8_t Val = Read(Addr);
            OpDEC(Addr, Val);
            Cycles = 6;
            break;
        }
        case 0xDE: { // DEC abs,X
            uint8_t AddrLow = Read(PC++);
            uint8_t AddrHigh = Read(PC++);
            uint16_t Addr = ((AddrHigh*256) + AddrLow) + X;
            uint8_t Val = Read(Addr);
            OpDEC(Addr, Val);
            Cycles = 7;
            break;
        }

        case 0x09: // ORA imm
            OpORA(A, Read(PC++));
            Cycles = 2;
            break;
        case 0x05: { // ORA zp
            uint8_t Addr = Read(PC++);
            OpORA(A, Read(Addr));
            Cycles = 3;
            break;
        }

        case 0x29: // AND imm
            OpAND(A, Read(PC++));
            Cycles = 2;
            break;
        case 0x25: { // AND zp
            uint8_t Addr = Read(PC++);
            OpAND(A, Read(Addr));
            Cycles = 3;
            break;
        }
        case 0x3D: { // AND abs,X
            uint8_t AddrLow = Read(PC++);
            uint8_t AddrHigh = Read(PC++);
            uint16_t Addr = (AddrHigh*256) + AddrLow + X;
            OpAND(A, Read(Addr));
            Cycles = 4;
            break;
        }

        case 0x49: // EOR imm
            OpEOR(A, Read(PC++));
            Cycles = 2;
            break;
        case 0x45: { // EOR zp
            uint8_t Addr = Read(PC++);
            OpEOR(A, Read(Addr));
            Cycles = 3;
            break;
        }

        case 0x69: // ADC imm
            OpADC(A, Read(PC++));
            Cycles = 2;
            break;
        case 0x65: { // ADC zp
            uint8_t Addr = Read(PC++);
            OpADC(A, Read(Addr));
            Cycles = 3;
            break;
        }
        case 0x6D: { // ADC abs
            uint8_t AddrLow = Read(PC++);
            uint8_t AddrHigh = Read(PC++);
            uint16_t Addr = (AddrHigh*256) + AddrLow;
            uint8_t Val = Read(Addr);
            OpADC(A, Val);
            Cycles = 4;
            break;
        }
        case 0x79: { // ADC abs,Y
            uint8_t AddrLow = Read(PC++);
            uint8_t AddrHigh = Read(PC++);
            uint16_t Addr = ((AddrHigh*256) + AddrLow + Y);
            uint8_t Val = Read(Addr);
            OpADC(A, Val);
            Cycles = 4;
            break;
        }
        case 0x75: { // ADC zp,X
            uint8_t Addr = (Read(PC++) + X) & 0xFF;
            uint8_t Val = Read(Addr);
            OpADC(A, Val);
            Cycles = 4;
            break;
        }

        case 0xE9: // SBC imm
            OpSBC(A, Read(PC++));
            Cycles = 2;
            break;
        case 0xE5: { // SBC zp
            uint8_t Addr = Read(PC++);
            OpSBC(A, Read(Addr));
            Cycles = 3;
            break;
        }
        case 0xF9: { // SBC abs,Y
            uint8_t AddrLow = Read(PC++);
            uint8_t AddrHigh = Read(PC++);
            uint16_t Addr = (AddrHigh*256) + AddrLow + Y;
            OpSBC(A, Read(Addr));
            Cycles = 4;
            break;
        }

        case 0xC9: // CMP imm
            OpCMP(A, Read(PC++));
            Cycles = 2;
            break;
        case 0xC5: { // CMP zp
            uint8_t Addr = Read(PC++);
            OpCMP(A, Read(Addr));
            Cycles = 3;
            break;
        }
        case 0xCD: { // CMP abs
            uint8_t AddrLow = Read(PC++);
            uint8_t AddrHigh = Read(PC++);
            uint16_t Addr = (AddrHigh*256) + AddrLow;
            OpCMP(A, Read(Addr));
            Cycles = 4;
            break;
        }
        case 0xD9: { // CMP abs,Y
            uint8_t AddrLow = Read(PC++);
            uint8_t AddrHigh = Read(PC++);
            uint16_t Addr = ((AddrHigh*256) + AddrLow + Y);
            OpCMP(A, Read(Addr));
            Cycles = 4;
            break;
        }

        case 0x24: { // BIT zp
            uint8_t Addr = Read(PC++);
            OpBIT(Read(Addr));
            Cycles = 3;
            break;
        }
        case 0x2C: { // BIT abs
            uint8_t ValLow = Read(PC++);
            uint8_t ValHigh = Read(PC++);
            uint16_t addr = (ValHigh*256) + ValLow;
            OpBIT(Read(addr));
            Cycles = 4;
            break;
        }

        case 0x4C: { // JMP abs
            uint8_t AddrLow = Read(PC++);
            uint8_t AddrHigh = Read(PC++);
            PC = (ushort)((AddrHigh*256) + AddrLow);
            Cycles = 3;
            break;
        }
        case 0x6C: { // JMP indirect
            uint8_t PtrLow = Read(PC++);
            uint8_t PtrHigh = Read(PC++);
            uint16_t Ptr = (PtrHigh*256) + PtrLow;
            uint8_t Low = Read(Ptr);
            uint8_t High = Read((Ptr & 0xFF00) | ((Ptr + 1) & 0xFF));
            PC = (ushort)((High*256) + Low);
            Cycles = 5;
            break;
        }

        case 0xE0: { // CPX imm
            uint8_t Val = Read(PC++);
            uint8_t Result = X - Val;
            FlagCarry = X >= Val;
            SetZN(Result);
            Cycles = 2;
            break;
        }
        case 0xC0: { // CPY imm
            uint8_t Val = Read(PC++);
            uint8_t Result = Y - Val;
            FlagCarry = Y >= Val;
            SetZN(Result);
            Cycles = 2;
            break;
        }

        case 0x00: { // BRK
            if (!DoNMI) {
                PC++;
            }
            Push((uint8_t)(PC >> 8));
            Push((uint8_t)PC);
            uint8_t Flags = 0;
            Flags += (FlagCarry ? 0x01 : 0);
            Flags += (FlagZero ? 0x02 : 0);
            Flags += (FlagInterruptDisable ? 0x04 : 0);
            Flags += (FlagDecimal ? 0x08 : 0);
            Flags += (DoNMI ? 0 : 0x10);
            Flags += 0x20;
            Flags += (FlagOverflow ? 0x40 : 0);
            Flags += (FlagNegative ? 0x80 : 0);
            Push(Flags);
            uint8_t AddrLow = Read(DoNMI ? 0xFFFA : 0xFFFE);
            uint8_t AddrHigh = Read(DoNMI ? 0xFFFB : 0xFFFF);
            PC = (ushort)((AddrHigh*256)+AddrLow);
            DoNMI = false;
            Cycles = 7;
            break;
        }
        case 0x40: { // RTI
            uint8_t Flags = Pull();
            FlagCarry = (Flags & 0x01) != 0;
            FlagZero = (Flags & 0x02) != 0;
            FlagInterruptDisable = (Flags & 0x04) != 0;
            FlagDecimal = (Flags & 0x08) != 0;
            FlagOverflow = (Flags & 0x40) != 0;
            FlagNegative = (Flags & 0x80) != 0;
            uint8_t PCL = Pull();
            uint8_t PCH = Pull();
            PC = (ushort)((PCH*256) + PCL);
            Cycles = 6;
            break;
        }

        default:
            printf("Unimplemented OP 0x%x\n", OpCode);
            exit(2);
            break;
    }
    while (Cycles > 0) {
        Cycles--;
        EmulatePPU();
        EmulatePPU();
        EmulatePPU();
    }
}

void IncScrollY(void) {
    if ((VRAMAddress & 0x7000) != 0x7000) {
        VRAMAddress += 0x1000;
    } else {
        VRAMAddress &= 0x0FFF;
        int YScroll = (VRAMAddress & 0x03E0) >> 5;
        if (YScroll == 29) {
            YScroll = 0;
            VRAMAddress ^= 0x0800;
        } else {
            YScroll++;
            YScroll &= 0x1F;
        }
        VRAMAddress = (ushort)((VRAMAddress & 0xFC1F) | (YScroll << 5));
    }
}

void ResetXScroll(void) {
    VRAMAddress = (ushort)((VRAMAddress & 0b0111101111100000) | (TransferAddress & 0b0000010000011111));
}

void ResetYScroll(void) {
    VRAMAddress = (ushort)((VRAMAddress & 0b0000010000011111) | (TransferAddress & 0b0111101111100000));
}

SDL_Surface *PPUBitmap = nullptr;

const uint32_t Palette[64] = {
    0xFF757575,0xFF271B8F,0xFF0000AB,0xFF47009F,0xFF8F0077,0xFFAB0013,0xFFA70000,0xFF7F0B00,
    0xFF432F00,0xFF004700,0xFF005100,0xFF003F17,0xFF1B3F5F,0xFF000000,0xFF000000,0xFF000000,
    0xFFBCBCBC,0xFF0073EF,0xFF233BEF,0xFF8300F3,0xFFBF00BF,0xFFE7005B,0xFFDB2B00,0xFFCB4F0F,
    0xFF8B7300,0xFF009F0F,0xFF00AB00,0xFF00933B,0xFF00838B,0xFF000000,0xFF000000,0xFF000000,
    0xFFFFFFFF,0xFF3FBFFF,0xFF5F97FF,0xFFA78BFD,0xFFF77BFF,0xFFFF77B7,0xFFFF7763,0xFFFF9B3B,
    0xFFF3BF3F,0xFF83D313,0xFF4FDF4B,0xFF58F898,0xFF00EBDB,0xFF000000,0xFF000000,0xFF000000,
    0xFFFFFFFF,0xFFA7E7FF,0xFFC7D7FF,0xFFD7CBFF,0xFFFFC7FF,0xFFFFC7DB,0xFFFFBFB3,0xFFFFDBAB,
    0xFFFFE7A3,0xFFE3FFA3,0xFFABF3BF,0xFFB3FFCF,0xFF9FFFF3,0xFF000000,0xFF000000,0xFF000000
};

void EmulatePPU(void) {
    if (Dot == 1 && Scanline == 241) {
        VBlank = true;
    } else if (Dot == 1 && Scanline == 261) {
        VBlank = false;
    }

    if ((Scanline < 240 || Scanline == 261)) {
        if ((Dot > 0 && Dot <= 256) || (Dot > 320 && Dot <= 336)) {
            if (MaskRenderBG || MaskRenderSprites) {
                if (MaskRenderBG) {
                    ShiftRegPatternL = ShiftRegPatternL << 1;
                    ShiftRegPatternH = ShiftRegPatternH << 1;
                    ShiftRegAttrL = ShiftRegAttrL << 1;
                    ShiftRegAttrH = ShiftRegAttrH << 1;

                    uint8_t CycleTick = (uint8_t)((Dot - 1) & 7);
                    switch (CycleTick) {
                        case 0:
                            ShiftRegPatternL = (ushort)((ShiftRegPatternL & 0xFF00) | EightStepPatternLowBitPlane);
                            ShiftRegPatternH = (ushort)((ShiftRegPatternH & 0xFF00) | EightStepPatternHighBitPlane);
                            ShiftRegAttrL = (ushort)((ShiftRegAttrL & 0xFF00) | ((EightStepAttr & 1) == 1 ? 0xFF : 0));
                            ShiftRegAttrH = (ushort)((ShiftRegAttrH & 0xFF00) | ((EightStepAttr & 2) == 2 ? 0xFF : 0));
                            AddrBus = 0x2000 + (VRAMAddress & 0x0FFF);
                            EightStepTemp = ReadPPU(AddrBus);
                            break;
                        case 1:
                            EightStepNextChar = EightStepTemp;
                            break;
                        case 2:
                            AddrBus = (0x23C0 | (VRAMAddress & 0x0C00) | ((VRAMAddress >> 4) & 0x38) | ((VRAMAddress >> 2) & 0x07));
                            EightStepTemp = ReadPPU(AddrBus);
                            break;
                        case 3:
                            EightStepAttr = EightStepTemp;
                            if ((VRAMAddress & 3) >= 2) {
                                EightStepAttr = EightStepAttr >> 2;
                            }
                            if ((((VRAMAddress & 0b0000001111100000) >> 5) & 3) >= 2) {
                                EightStepAttr = EightStepAttr >> 4;
                            }
                            EightStepAttr = EightStepAttr & 3;
                            break;
                        case 4:
                            AddrBus = (((VRAMAddress & 0b0111000000000000) >> 12) | EightStepNextChar * 16 | (BGPatternTable ? 0x1000 : 0));
                            EightStepTemp = ReadPPU(AddrBus);
                            break;
                        case 5:
                            EightStepPatternLowBitPlane = EightStepTemp;
                            AddrBus += 8;
                            break;
                        case 6:
                            EightStepTemp = ReadPPU(AddrBus);
                            break;
                        case 7:
                            EightStepPatternHighBitPlane = EightStepTemp;
                            if ((VRAMAddress & 0x001F) == 31) {
                                VRAMAddress &= 0xFFE0;
                                VRAMAddress ^= 0x0400;
                            } else {
                                VRAMAddress++;
                            }
                            break;
                    }
                }
            }
        }
    }
    if (Dot == 256) {
        IncScrollY();
    } else if (Dot == 257) {
        ResetXScroll();
    }
    if (Dot >= 280 && Dot <= 304 && Scanline == 261) {
        ResetYScroll();
    }
    if (Scanline < 241 && Dot > 0 && Dot <= 256) {
        uint8_t PalHigh = 0;
        uint8_t PalLow = 0;
        if (MaskRenderBG && (Dot > 8 || Mask8pxMaskBG)) {
            uint8_t Col0 = (uint8_t)(((ShiftRegPatternL >> (15 - ScrollFineX))) & 1);
            uint8_t Col1 = (uint8_t)(((ShiftRegPatternH >> (15 - ScrollFineX))) & 1);
            PalLow = ((Col1 << 1) | Col0);

            uint8_t Pal0 = (uint8_t)(((ShiftRegAttrL) >> (15 - ScrollFineX)) & 1);
            uint8_t Pal1 = (uint8_t)(((ShiftRegAttrH) >> (15 - ScrollFineX)) & 1);
            PalHigh = ((Pal1 << 1) | Pal0);

            if (PalLow == 0 && PalHigh != 0) {
                PalHigh = 0;
            }

            uint8_t PalIndex = (PalHigh << 2) | PalLow; // 0–63

            // Write pixel to SDL_Surface
            if (PPUBitmap && PPUBitmap->pixels) {
                uint32_t* Pixels = (uint32_t*)PPUBitmap->pixels;
                int X = Dot - 1;          // 0-based pixel X
                int Y = Scanline;         // 0-based pixel Y
                Pixels[Y * PPUBitmap->w + X] = Palette[PalIndex];
            }
        }
    }
    Dot++;
    if (Dot > 341) {
        Dot = 0;
        Scanline++;
        if (Scanline > 261) {
            Scanline = 0;
        }
    }
}

int64_t RunTimer = 0;

void Run(void) {
    while (!CPUHalted) {
        EmulateCPU();
        RunTimer++;

        if (RunTimer>500000) {
            CPUHalted = true;
        }
    }
}

int main(int argc, char **argv) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* Win = SDL_CreateWindow("awesome emulator", 100, 100, NES_WIDTH, NES_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    SDL_Renderer* Renderer = SDL_CreateRenderer(Win, -1, SDL_RENDERER_ACCELERATED);
    SDL_Surface* Bitmap = SDL_CreateRGBSurfaceWithFormat(0, NES_WIDTH, NES_HEIGHT, 32, SDL_PIXELFORMAT_ARGB8888);
    PPUBitmap = Bitmap;
    Reset(argv[1]);
    Run();

    /*for (int Row = 0; Row < 30; Row++) {
        for (int Col = 0; Col < 32; Col++) {
            uint8_t AttrOffset = ((Col >> 2) + (Row >> 2) * 8);
            uint8_t Attrs = VRAM[0x3C0 + AttrOffset];
            uint8_t Quad = (((Col >> 1) & 1) + ((Row >> 1) & 1) * 2);
            uint8_t Pair = ((Attrs >> (Quad * 2)) & 3);
            for (int ScreenY = 0; ScreenY < 8; ScreenY++) {
                int Use2ndPatternTable = BGPatternTable ? 4096 : 0;
                uint8_t LowByte = CHRData[VRAM[Col + Row * 32] * 16 + ScreenY+Use2ndPatternTable];
                uint8_t HighByte = CHRData[VRAM[Col + Row * 32] * 16 + 8 + ScreenY+Use2ndPatternTable];
                for (int ScreenX = 0; ScreenX < 8; ScreenX++) {
                    // draw pixel
                    int TwoBit = ((LowByte >> (7-ScreenX)) & 1) == 1 ? 1 : 0;
                    TwoBit += ((HighByte >> (7-ScreenX)) & 1) == 1 ? 2 : 0;
                    uint32_t Color = 0;
                    if (TwoBit == 0) {
                        Color = Palette[PaletteRAM[0]];
                    } else {
                        Color = Palette[PaletteRAM[TwoBit + Pair * 4]];
                    }
                    ((uint32_t*)Bitmap->pixels)[(ScreenY + Row * 8) * Bitmap->w + (ScreenX + Col * 8)] = Color;
                }
            }
        }
    }*/
    SDL_Texture* Tex = SDL_CreateTextureFromSurface(Renderer, Bitmap);
    SDL_FreeSurface(Bitmap);

    bool CanRun = true;
    SDL_Event E;
    while (CanRun) {
        while (SDL_PollEvent(&E)) {
            if (E.type == SDL_QUIT) CanRun = false;
        }
        SDL_RenderClear(Renderer);
        SDL_RenderCopy(Renderer, Tex, nullptr, nullptr);
        SDL_RenderPresent(Renderer);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(Tex);
    SDL_DestroyRenderer(Renderer);
    SDL_DestroyWindow(Win);
    SDL_Quit();
    return 0;
}