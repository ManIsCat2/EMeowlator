#include "savestate.hpp"
#include "nes/nes_cpu.hpp"
#include "nes/nes_ppu.hpp"
#include "nes/nes_apu.hpp"

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

void SaveStateFile::Write(const char *FileName) {
    OpenFileW(FileName);
    
    WriteBytes<uint32_t>(NYA_SIGNATURE);
    WriteBytes<uint16_t>(getNESRom()->MapperID);
    WriteBytesPtr<uint8_t>(cpu.RAM, RAM_SIZE);

    uint8_t A,X,Y,SP,P = 0;
    uint16_t PC = 0;
    cpu.GetInfo(&A,&X,&Y,&PC,&SP,&P);
    WriteBytes<uint8_t>(A);
    WriteBytes<uint8_t>(X);
    WriteBytes<uint8_t>(Y);
    WriteBytes<uint16_t>(PC);
    WriteBytes<uint8_t>(SP);
    WriteBytes<uint8_t>(P);

    WriteBytes<bool>(false); //cpu.CPUPaused
    WriteBytes<bool>(cpu.NMIDetector);
    WriteBytes<bool>(cpu.doNMI);
    WriteBytes<bool>(cpu.doIRQ);
    WriteBytes<bool>(cpu.IRQPending);

    WriteBytesPtr<uint8_t>(ppu.VRAM.data(), VRAM_SIZE);
    WriteBytesPtr<uint8_t>(ppu.paletteRAM.data(), PALRAM_SIZE);

    WriteBytesPtr<uint8_t>(ppu.OAM, 0x100);

    WriteBytes<bool>(ppu.WriteLatch);
    WriteBytes<uint16_t>(ppu.VRAMAddr);
    WriteBytes<uint16_t>(ppu.OAMAddr);
    WriteBytes<uint16_t>(ppu.TransferAddr);
    WriteBytes<uint8_t>(ppu.ReadBuffer);
    WriteBytes<int>(ppu.Dot);
    WriteBytes<int>(ppu.ScanLine);
    WriteBytes<bool>(ppu.Vblank);
    WriteBytes<bool>(ppu.sprite0Hit);
    WriteBytes<bool>(ppu.spriteOverflow);
    WriteBytes<bool>(ppu.DisableSprites);
    WriteBytes<bool>(ppu.VRAMCorruption);

    WriteBytes<bool>(ppu.mask.grayscaleMode);
    WriteBytes<bool>(ppu.mask.background8pxMask);
    WriteBytes<bool>(ppu.mask.sprite8pxMask);
    WriteBytes<bool>(ppu.mask.renderBackground);
    WriteBytes<bool>(ppu.mask.renderSprites);
    WriteBytes<uint8_t>(ppu.mask.combined);

    WriteBytes<int>(ppu.control.nametableSelect);
    WriteBytes<bool>(ppu.control.VRAMInc32);
    WriteBytes<int>(ppu.control.spritePatternTable);
    WriteBytes<int>(ppu.control.BGPatternTable);
    WriteBytes<bool>(ppu.control.use8x16Sprites);
    WriteBytes<bool>(ppu.control.enableNMI);
    WriteBytes<uint8_t>(ppu.control.combined);

    WriteBytes<uint8_t>(ppu.scrollFineX);
    WriteBytes<uint8_t>(ppu.patternTableLow);
    WriteBytes<uint8_t>(ppu.patternTableHigh);
    WriteBytes<uint8_t>(ppu.nametableByte);
    WriteBytes<uint16_t>(ppu.shiftRegHigh);
    WriteBytes<uint16_t>(ppu.shiftRegLow);
    WriteBytes<uint16_t>(ppu.shiftAttrHigh);
    WriteBytes<uint16_t>(ppu.shiftAttrLow);
    WriteBytes<uint16_t>(ppu.attributeByte);

    if (getNESRom()->CHRRomSize == 0) {
        WriteBytesPtr<uint8_t>(ppu.ChrData.data(), 0x2000);
    }

    WriteBytes<bool>(apu.IRQPending);
    WriteBytes<bool>(apu.DMCIrqPending);
    WriteBytes<bool>(apu.DMCIrqEnable);
    WriteBytes<bool>(apu.IRQInhibit);

    WriteBytes<float>(apu.pulse1Volume);
    WriteBytes<float>(apu.pulse2Volume);
    WriteBytes<float>(apu.triangleVolume);
    WriteBytes<float>(apu.noiseVolume);
    WriteBytes<float>(apu.dmcVolume);
    WriteBytes<float>(apu.masterVolume);

    WriteBytes<uint32_t>(apu.clockCounter);
    WriteBytes<uint32_t>(apu.frameCounter);
    WriteBytes<uint8_t>(apu.frameMode);
    WriteBytes<int>(apu.frameCounterResetDelay);
    WriteBytes<uint8_t>(apu.delayedFrameMode);

    auto writePulse = [&](const PulseChannel& p){
        WriteBytes<bool>(p.enable);
        WriteBytes<uint8_t>(p.duty);
        WriteBytes<uint8_t>(p.dutySeq);
        WriteBytes<uint16_t>(p.timer);
        WriteBytes<uint16_t>(p.timerReload);
        WriteBytes<uint8_t>(p.lengthCounter);
        WriteBytes<bool>(p.lengthHalt);
        WriteBytes<bool>(p.constantVolume);
        WriteBytes<uint8_t>(p.volume);
        WriteBytes<bool>(p.envStart);
        WriteBytes<uint8_t>(p.envVol);
        WriteBytes<uint8_t>(p.envDivider);
        WriteBytes<bool>(p.sweepEnable);
        WriteBytes<uint8_t>(p.sweepPeriod);
        WriteBytes<bool>(p.sweepNegate);
        WriteBytes<uint8_t>(p.sweepShift);
        WriteBytes<bool>(p.sweepReload);
        WriteBytes<uint8_t>(p.sweepDivider);
    };
    writePulse(apu.pulse1);
    writePulse(apu.pulse2);

    WriteBytes<bool>(apu.triangle.enable);
    WriteBytes<bool>(apu.triangle.lengthHalt);
    WriteBytes<uint8_t>(apu.triangle.linearCounter);
    WriteBytes<uint8_t>(apu.triangle.linearReload);
    WriteBytes<bool>(apu.triangle.linearReloadFlag);
    WriteBytes<uint16_t>(apu.triangle.timer);
    WriteBytes<uint16_t>(apu.triangle.timerReload);
    WriteBytes<uint8_t>(apu.triangle.lengthCounter);
    WriteBytes<uint8_t>(apu.triangle.dutySeq);

    WriteBytes<bool>(apu.noise.enable);
    WriteBytes<bool>(apu.noise.lengthHalt);
    WriteBytes<bool>(apu.noise.constantVolume);
    WriteBytes<uint8_t>(apu.noise.volume);
    WriteBytes<uint16_t>(apu.noise.timer);
    WriteBytes<uint16_t>(apu.noise.timerReload);
    WriteBytes<uint8_t>(apu.noise.lengthCounter);
    WriteBytes<uint16_t>(apu.noise.shiftRegister);
    WriteBytes<bool>(apu.noise.mode);
    WriteBytes<bool>(apu.noise.envStart);
    WriteBytes<uint8_t>(apu.noise.envVol);
    WriteBytes<uint8_t>(apu.noise.envDivider);

    WriteBytes<bool>(apu.dmc.enable);
    WriteBytes<bool>(apu.dmc.loop);
    WriteBytes<uint16_t>(apu.dmc.timer);
    WriteBytes<uint16_t>(apu.dmc.timerReload);
    WriteBytes<uint16_t>(apu.dmc.currentAddress);
    WriteBytes<uint16_t>(apu.dmc.sampleAddress);
    WriteBytes<uint16_t>(apu.dmc.currentLength);
    WriteBytes<uint16_t>(apu.dmc.sampleLength);
    WriteBytes<uint8_t>(apu.dmc.shiftRegister);
    WriteBytes<uint8_t>(apu.dmc.bitsRemaining);
    WriteBytes<uint8_t>(apu.dmc.sampleBuffer);
    WriteBytes<bool>(apu.dmc.sampleBufferEmpty);
    WriteBytes<uint8_t>(apu.dmc.outputLevel);
    WriteBytes<bool>(apu.dmc.silence);

    uint32_t SRAMSize = getNESRom()->mapper->getSRAMSize();
    if (getNESRom()->hasBattery && SRAMSize != 0) {
        WriteBytesPtr<uint8_t>(getNESRom()->mapper->SRAM, SRAMSize);
    } else {
        WriteBytesPtr<uint8_t>(getNESRom()->mapper->PRGRam, 0x2000);
    }

    getNESRom()->mapper->saveState(*this);

    CloseFile();
}

void SaveStateFile::Load(const char *FileName) {
    OpenFileR(FileName);

    uint32_t detectedSig = ReadBytes<uint32_t>();
    if (detectedSig != NYA_SIGNATURE) {
        DebugPrintLog("SAVESTATE", "File has invalid MeowNES Savestate header");
        QMessageBox::critical((QMainWindow*)globalQTWin, "Error", "File has invalid MeowNES Savestate header");
        CloseFile();
        return;
    }

    uint16_t mapperID = ReadBytes<uint16_t>();
    if (mapperID != getNESRom()->MapperID) {
        DebugPrintLog("SAVESTATE", "State used in incorrect ROM");

        QMessageBox::StandardButton reply;

        reply = QMessageBox::question(
            (QMainWindow*)globalQTWin,
            "Mapper mismatch",
            "This savestate wasn't meant to be used with this ROM\n\nDo you want to continue?",
            QMessageBox::Yes | QMessageBox::No
        );

        if (reply == QMessageBox::Yes) {
            // continue
        } else {
            CloseFile();
            return;
        }
    }
    ReadBytesPtr<uint8_t>(cpu.RAM, RAM_SIZE);

    uint8_t A,X,Y,SP,P = 0;
    uint16_t PC = 0;
    A = ReadBytes<uint8_t>();
    X = ReadBytes<uint8_t>();
    Y = ReadBytes<uint8_t>();
    PC = ReadBytes<uint16_t>();
    SP = ReadBytes<uint8_t>();
    P  = ReadBytes<uint8_t>();
    cpu.SetInfo(A,X,Y,PC,SP,P);

    cpu.CPUPaused      = ReadBytes<bool>();
    cpu.NMIDetector    = ReadBytes<bool>();
    cpu.doNMI          = ReadBytes<bool>();
    cpu.doIRQ          = ReadBytes<bool>();
    cpu.IRQPending     = ReadBytes<bool>();

    ReadBytesPtr<uint8_t>(ppu.VRAM.data(), VRAM_SIZE);
    ReadBytesPtr<uint8_t>(ppu.paletteRAM.data(), PALRAM_SIZE);
    ReadBytesPtr<uint8_t>(ppu.OAM, 0x100);

    ppu.WriteLatch      = ReadBytes<bool>();
    ppu.VRAMAddr        = ReadBytes<uint16_t>();
    ppu.OAMAddr         = ReadBytes<uint16_t>();
    ppu.TransferAddr    = ReadBytes<uint16_t>();
    ppu.ReadBuffer      = ReadBytes<uint8_t>();
    ppu.Dot             = ReadBytes<int>();
    ppu.ScanLine        = ReadBytes<int>();
    ppu.Vblank          = ReadBytes<bool>();
    ppu.sprite0Hit      = ReadBytes<bool>();
    ppu.spriteOverflow  = ReadBytes<bool>();
    ppu.DisableSprites  = ReadBytes<bool>();
    ppu.VRAMCorruption  = ReadBytes<bool>();

    ppu.mask.grayscaleMode   = ReadBytes<bool>();
    ppu.mask.background8pxMask = ReadBytes<bool>();
    ppu.mask.sprite8pxMask    = ReadBytes<bool>();
    ppu.mask.renderBackground = ReadBytes<bool>();
    ppu.mask.renderSprites    = ReadBytes<bool>();
    ppu.mask.combined         = ReadBytes<uint8_t>();

    ppu.control.nametableSelect = ReadBytes<int>();
    ppu.control.VRAMInc32       = ReadBytes<bool>();
    ppu.control.spritePatternTable = ReadBytes<int>();
    ppu.control.BGPatternTable      = ReadBytes<int>();
    ppu.control.use8x16Sprites      = ReadBytes<bool>();
    ppu.control.enableNMI           = ReadBytes<bool>();
    ppu.control.combined            = ReadBytes<uint8_t>();

    ppu.scrollFineX        = ReadBytes<uint8_t>();
    ppu.patternTableLow    = ReadBytes<uint8_t>();
    ppu.patternTableHigh   = ReadBytes<uint8_t>();
    ppu.nametableByte      = ReadBytes<uint8_t>();
    ppu.shiftRegHigh       = ReadBytes<uint16_t>();
    ppu.shiftRegLow        = ReadBytes<uint16_t>();
    ppu.shiftAttrHigh      = ReadBytes<uint16_t>();
    ppu.shiftAttrLow       = ReadBytes<uint16_t>();
    ppu.attributeByte      = ReadBytes<uint16_t>();

    if (getNESRom()->CHRRomSize == 0) {
        ReadBytesPtr<uint8_t>(ppu.ChrData.data(), 0x2000);
    }

    apu.IRQPending       = ReadBytes<bool>();
    apu.DMCIrqPending    = ReadBytes<bool>();
    apu.DMCIrqEnable     = ReadBytes<bool>();
    apu.IRQInhibit       = ReadBytes<bool>();

    apu.pulse1Volume  = ReadBytes<float>();
    apu.pulse2Volume  = ReadBytes<float>();
    apu.triangleVolume= ReadBytes<float>();
    apu.noiseVolume   = ReadBytes<float>();
    apu.dmcVolume     = ReadBytes<float>();
    apu.masterVolume  = ReadBytes<float>();

    apu.clockCounter          = ReadBytes<uint32_t>();
    apu.frameCounter          = ReadBytes<uint32_t>();
    apu.frameMode             = ReadBytes<uint8_t>();
    apu.frameCounterResetDelay= ReadBytes<int>();
    apu.delayedFrameMode      = ReadBytes<uint8_t>();

    auto readPulse = [&](PulseChannel& p){
        p.enable           = ReadBytes<bool>();
        p.duty             = ReadBytes<uint8_t>();
        p.dutySeq          = ReadBytes<uint8_t>();
        p.timer            = ReadBytes<uint16_t>();
        p.timerReload      = ReadBytes<uint16_t>();
        p.lengthCounter    = ReadBytes<uint8_t>();
        p.lengthHalt       = ReadBytes<bool>();
        p.constantVolume    = ReadBytes<bool>();
        p.volume           = ReadBytes<uint8_t>();
        p.envStart         = ReadBytes<bool>();
        p.envVol           = ReadBytes<uint8_t>();
        p.envDivider       = ReadBytes<uint8_t>();
        p.sweepEnable      = ReadBytes<bool>();
        p.sweepPeriod      = ReadBytes<uint8_t>();
        p.sweepNegate      = ReadBytes<bool>();
        p.sweepShift       = ReadBytes<uint8_t>();
        p.sweepReload      = ReadBytes<bool>();
        p.sweepDivider     = ReadBytes<uint8_t>();
    };
    readPulse(apu.pulse1);
    readPulse(apu.pulse2);

    apu.triangle.enable           = ReadBytes<bool>();
    apu.triangle.lengthHalt      = ReadBytes<bool>();
    apu.triangle.linearCounter   = ReadBytes<uint8_t>();
    apu.triangle.linearReload    = ReadBytes<uint8_t>();
    apu.triangle.linearReloadFlag= ReadBytes<bool>();
    apu.triangle.timer           = ReadBytes<uint16_t>();
    apu.triangle.timerReload     = ReadBytes<uint16_t>();
    apu.triangle.lengthCounter   = ReadBytes<uint8_t>();
    apu.triangle.dutySeq         = ReadBytes<uint8_t>();

    apu.noise.enable             = ReadBytes<bool>();
    apu.noise.lengthHalt         = ReadBytes<bool>();
    apu.noise.constantVolume     = ReadBytes<bool>();
    apu.noise.volume             = ReadBytes<uint8_t>();
    apu.noise.timer              = ReadBytes<uint16_t>();
    apu.noise.timerReload        = ReadBytes<uint16_t>();
    apu.noise.lengthCounter      = ReadBytes<uint8_t>();
    apu.noise.shiftRegister      = ReadBytes<uint16_t>();
    apu.noise.mode               = ReadBytes<bool>();
    apu.noise.envStart           = ReadBytes<bool>();
    apu.noise.envVol             = ReadBytes<uint8_t>();
    apu.noise.envDivider         = ReadBytes<uint8_t>();

    apu.dmc.enable               = ReadBytes<bool>();
    apu.dmc.loop                 = ReadBytes<bool>();
    apu.dmc.timer                = ReadBytes<uint16_t>();
    apu.dmc.timerReload          = ReadBytes<uint16_t>();
    apu.dmc.currentAddress       = ReadBytes<uint16_t>();
    apu.dmc.sampleAddress        = ReadBytes<uint16_t>();
    apu.dmc.currentLength        = ReadBytes<uint16_t>();
    apu.dmc.sampleLength         = ReadBytes<uint16_t>();
    apu.dmc.shiftRegister        = ReadBytes<uint8_t>();
    apu.dmc.bitsRemaining        = ReadBytes<uint8_t>();
    apu.dmc.sampleBuffer         = ReadBytes<uint8_t>();
    apu.dmc.sampleBufferEmpty    = ReadBytes<bool>();
    apu.dmc.outputLevel          = ReadBytes<uint8_t>();
    apu.dmc.silence              = ReadBytes<bool>();

    uint32_t SRAMSize = getNESRom()->mapper->getSRAMSize();
    if (getNESRom()->hasBattery && SRAMSize != 0) {
        ReadBytesPtr<uint8_t>(getNESRom()->mapper->SRAM, SRAMSize);
    } else {
        ReadBytesPtr<uint8_t>(getNESRom()->mapper->PRGRam, 0x2000);
    }

    getNESRom()->mapper->loadState(*this);

    DebugPrintLog("SAVESTATE", "Loaded Savestate '%s'", FileName);
    CloseFile();
}