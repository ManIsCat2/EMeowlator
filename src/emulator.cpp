#include "emulator.hpp"

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

uint8_t RAM[0x800] = {0};
uint8_t ROM[0x8000] = {0};
uint8_t Header[0x10] = {0};

bool CPUHalted = false;

uint8_t Read(ushort Address) {
    if (Address < 0x800) {
        return RAM[Address];
    }
    if (Address >= 0x8000) {
        return ROM[Address-0x8000];
    }
    return 0;
}

void Write(ushort Address, uint8_t Value) {
    if (Address < 0x800) {
        RAM[Address] = Value;
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

void EmulateCPU(void) {
    uint8_t Cycles = 0;
    uint8_t OpCode = Read(PC++);
    switch (OpCode) {
        case 0x02: // HLT
            CPUHalted = true;
            printf("Halt on 0x%x", PC);
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
        case 0xAD: // LDA abs
            LoadAbs(A);
            Cycles = 3;
            break;
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
        case 0x8E: // STX abs
            StoreAbs(X);
            Cycles = 4;
            break;

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


        default:
            printf("Unimplemented 0x%x\n", OpCode);
            break;
    }
}

void Run(void) {
    while (!CPUHalted) {
        EmulateCPU();
    }
}

int main(int argc, char **argv) {
    Reset(argv[1]);
    Run();
    printf("\nRAM: 0x%x\n", RAM[0]);
    return 0;
}