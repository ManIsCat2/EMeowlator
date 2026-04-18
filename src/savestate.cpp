#include "savestate.hpp"
#include "nes/nes_cpu.hpp"
#include "nes/nes_ppu.hpp"

void SaveStateFile::OpenFileR(const char *Name) {
    File = fopen(Name, "rb");
    if (!File) {
        DebugPrintLog("SAVESTATE", "Can't open \"%s\"", Name);
        exit(1);
    }
    fseek(File, 0, SEEK_END);
    FileSize = ftell(File);
    fseek(File, 0, SEEK_SET);
    Data = (uint8_t *) malloc(FileSize);
    fread(Data, 1, FileSize, File);
    ReadOnly = true;
}

void SaveStateFile::OpenFileW(const char *Name) {
    File = fopen(Name, "wb");
    if (!File) {
        DebugPrintLog("SAVESTATE", "Can't open \"%s\"", Name);
        exit(1);
    }
    ReadOnly = false;
}

void SaveStateFile::CloseFile(void) {
    if (ReadOnly) free(Data);
    fclose(File);
}

void SaveStateFile::WriteSaveStateToFile(const char *FileName) {
    OpenFileW(FileName);

    //cpu
    WriteBytesPtr<uint8_t>(cpu.RAM, RAM_SIZE);
    uint8_t A,X,Y,SP,P = 0;
    uint16_t PC = 0;
    cpu.GetInfo(&A, &X, &Y, &PC, &SP, &P);
    WriteBytes<uint8_t>(A);
    WriteBytes<uint8_t>(X);
    WriteBytes<uint8_t>(Y);
    WriteBytes<uint16_t>(PC);
    WriteBytes<uint8_t>(SP);
    WriteBytes<uint8_t>(P);

    //ppu
    WriteBytesPtr<uint8_t>(ppu.VRAM.data(), 0x4000);
    WriteBytesPtr<uint8_t>(ppu.paletteRAM.data(), 0x20);
    WriteBytesPtr<uint8_t>(ppu.OAM, 0x100);

    WriteBytes<bool>(ppu.WriteLatch);
    WriteBytes<uint16_t>(ppu.VRAMAddr);
    WriteBytes<uint16_t>(ppu.OAMAddr);
    WriteBytes<uint16_t>(ppu.TransferAddr);
    WriteBytes<uint8_t>(ppu.ReadBuffer);
    WriteBytes<int>(ppu.Dot);
    WriteBytes<int>(ppu.ScanLine);
    WriteBytes<bool>(ppu.Vblank);

    WriteBytes<bool>(ppu.mask.background8pxMask);
    WriteBytes<bool>(ppu.mask.sprite8pxMask);
    WriteBytes<bool>(ppu.mask.renderBackground);
    WriteBytes<bool>(ppu.mask.renderSprites);

    WriteBytes<int>(ppu.control.nametableSelect);
    WriteBytes<bool>(ppu.control.VRAMInc32);
    WriteBytes<int>(ppu.control.spritePatternTable);
    WriteBytes<int>(ppu.control.BGPatternTable);
    WriteBytes<bool>(ppu.control.use8x16Sprites);
    WriteBytes<bool>(ppu.control.enableNMI);

    WriteBytes<uint8_t>(ppu.scrollFineX);

    CloseFile();
}

void SaveStateFile::LoadSaveStateFromFile(const char *FileName) {
    OpenFileR(FileName);

    //cpu
    ReadBytesPtr<uint8_t>(cpu.RAM, RAM_SIZE);
    uint8_t A,X,Y,SP,P = 0;
    uint16_t PC = 0;
    A = ReadBytes<uint8_t>();
    X = ReadBytes<uint8_t>();
    Y = ReadBytes<uint8_t>();
    PC = ReadBytes<uint16_t>();
    SP = ReadBytes<uint8_t>();
    P = ReadBytes<uint8_t>();
    cpu.SetInfo(A, X, Y, PC, SP, P);

    // ppu
    ReadBytesPtr<uint8_t>(ppu.VRAM.data(), 0x4000);
    ReadBytesPtr<uint8_t>(ppu.paletteRAM.data(), 0x20);
    ReadBytesPtr<uint8_t>(ppu.OAM, 0x100);

    ppu.WriteLatch      = ReadBytes<bool>();
    ppu.VRAMAddr        = ReadBytes<uint16_t>();
    ppu.OAMAddr         = ReadBytes<uint16_t>();
    ppu.TransferAddr    = ReadBytes<uint16_t>();
    ppu.ReadBuffer      = ReadBytes<uint8_t>();
    ppu.Dot             = ReadBytes<int>();
    ppu.ScanLine        = ReadBytes<int>();
    ppu.Vblank          = ReadBytes<bool>();

    ppu.mask.background8pxMask   = ReadBytes<bool>();
    ppu.mask.sprite8pxMask = ReadBytes<bool>();
    ppu.mask.renderBackground    = ReadBytes<bool>();
    ppu.mask.renderSprites = ReadBytes<bool>();

    ppu.control.nametableSelect = ReadBytes<int>();
    ppu.control.VRAMInc32   = ReadBytes<bool>();
    ppu.control.spritePatternTable = ReadBytes<int>();
    ppu.control.BGPatternTable  = ReadBytes<int>();
    ppu.control.use8x16Sprites  = ReadBytes<bool>();
    ppu.control.enableNMI       = ReadBytes<bool>();

    ppu.scrollFineX     = ReadBytes<uint8_t>();

    DebugPrintLog("SAVESTATE", "Loaded Savestate '%s'", FileName);

    CloseFile();
}
