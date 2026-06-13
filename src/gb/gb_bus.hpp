#pragma once

class GbCPU;
class GbPPU;

class HasGBBus {
public:
    GbCPU *cpu = nullptr;
    GbPPU *ppu = nullptr;
    void connectBus(GbCPU *sysCPU, GbPPU *sysPPU) {
        cpu = sysCPU;
        ppu = sysPPU;
    }
};
