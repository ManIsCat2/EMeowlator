#include "savestate.hpp"
#include "nes_cpu.hpp"
#include "nes_ppu.hpp"

void SaveStateFile::OpenFileR(const char *Name) {
    File = fopen(Name, "rb");
    if (!File) {
        printf("Can't open \"%s\"\n", Name);
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
        printf("Can't open \"%s\"\n", Name);
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
    WriteBytesPtr<uint8_t>(ppu.OAM.data(), 0x100);

    WriteBytes<bool>(ppu.WriteLatch);
    WriteBytes<unsigned short>(ppu.VRAMAddr);
    WriteBytes<unsigned short>(ppu.TransferAddr);
    WriteBytes<unsigned short>(ppu.OAMAddr);
    WriteBytes<unsigned short>(ppu.TempVRAMAddr);
    WriteBytes<uint8_t>(ppu.ReadBuffer);
    WriteBytes<int>(ppu.Dot);
    WriteBytes<int>(ppu.ScanLine);
    WriteBytes<bool>(ppu.Vblank);

    WriteBytes<bool>(ppu.mask8pxMaskBG);
    WriteBytes<bool>(ppu.mask8pxMaskSprites);
    WriteBytes<bool>(ppu.maskRenderBG);
    WriteBytes<bool>(ppu.maskRenderSprites);

    WriteBytes<int>(ppu.nametableSelect);
    WriteBytes<bool>(ppu.VRAMInc32Mode);
    WriteBytes<bool>(ppu.spritePatternTable);
    WriteBytes<bool>(ppu.BGPatternTable);
    WriteBytes<bool>(ppu.use8x16Sprites);
    WriteBytes<bool>(ppu.enableNMI);

    WriteBytes<uint16_t>(ppu.scrollX);
    WriteBytes<uint16_t>(ppu.scrollY);
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
    ReadBytesPtr<uint8_t>(ppu.OAM.data(), 0x100);

    ppu.WriteLatch      = ReadBytes<bool>();
    ppu.VRAMAddr        = ReadBytes<unsigned short>();
    ppu.TransferAddr    = ReadBytes<unsigned short>();
    ppu.OAMAddr         = ReadBytes<unsigned short>();
    ppu.TempVRAMAddr    = ReadBytes<unsigned short>();
    ppu.ReadBuffer      = ReadBytes<uint8_t>();
    ppu.Dot             = ReadBytes<int>();
    ppu.ScanLine        = ReadBytes<int>();
    ppu.Vblank          = ReadBytes<bool>();

    ppu.mask8pxMaskBG   = ReadBytes<bool>();
    ppu.mask8pxMaskSprites = ReadBytes<bool>();
    ppu.maskRenderBG    = ReadBytes<bool>();
    ppu.maskRenderSprites = ReadBytes<bool>();

    ppu.nametableSelect = ReadBytes<int>();
    ppu.VRAMInc32Mode   = ReadBytes<bool>();
    ppu.spritePatternTable = ReadBytes<bool>();
    ppu.BGPatternTable  = ReadBytes<bool>();
    ppu.use8x16Sprites  = ReadBytes<bool>();
    ppu.enableNMI       = ReadBytes<bool>();

    ppu.scrollX         = ReadBytes<uint16_t>();
    ppu.scrollY         = ReadBytes<uint16_t>();
    ppu.scrollFineX     = ReadBytes<uint8_t>();

    CloseFile();
}
