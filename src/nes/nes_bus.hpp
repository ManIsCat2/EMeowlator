#pragma once

class NesCPU;
class NesPPU;
class NesAPU;

class HasNESBus {
public:
    NesCPU *cpu = nullptr;
    NesPPU *ppu = nullptr;
    NesAPU *apu = nullptr;
    void connectBus(NesCPU *sysCPU, NesPPU *sysPPU, NesAPU *sysAPU) {
        cpu = sysCPU;
        ppu = sysPPU;
        apu = sysAPU;
    }
};
