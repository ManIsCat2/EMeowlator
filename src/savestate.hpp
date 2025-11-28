#pragma once

#include <stdio.h>
#include <string.h>
#include <string>
#include <stdint.h>

class SaveStateFile {
public:
    FILE *File;
    int32_t FileSize = 0;
    uint8_t *Data;
    int32_t Offset = 0;
    bool ReadOnly = true;

    void OpenFileR(const char *Name);
    void OpenFileW(const char *Name);

    template <typename T>
    T ReadBytes() {
        T Buf{};
        constexpr size_t RealSize = sizeof(T);
        if (Offset + RealSize > FileSize) return Buf;
        memcpy(&Buf, Data + Offset, RealSize);
        Offset += RealSize;
        return Buf;
    }

    template <typename T>
    T *ReadBytesPtr(T *Buf, uint32_t Len) {
        size_t RealSize = sizeof(T) * Len;
        if (Offset + RealSize > FileSize) return Buf;
        memcpy(Buf, Data + Offset, RealSize);
        Offset += RealSize;
        return Buf;
    }

    template <typename T>
    bool WriteBytes(T Value) {
        size_t Written = fwrite(&Value, sizeof(T), 1, File);
        return Written == 1;
    }

    template <typename T>
    bool WriteBytesPtr(T *Buf, uint32_t Len) {
        size_t Written = fwrite(Buf, sizeof(T), Len, File);
        return Written == Len;
    }

    void CloseFile(void);
    void WriteSaveStateToFile(const char *FileName);
    void LoadSaveStateFromFile(const char *FileName);
};