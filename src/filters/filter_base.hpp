#pragma once

#include <nes.hpp>

#include <stdint.h>
#include <string>

class VFilterBase {
public:
    virtual ~VFilterBase() {};
    virtual void initialize(void) {};
    virtual void applyFilter(UNUSED uint32_t *pix, UNUSED int x, UNUSED int y) {};
    virtual bool hasCustomBlit(void) { return false; };
    virtual void blit(void) {};
};

enum class VideoFilter {
    NONE,
    NTSC,
    CHROMA,
    GRAYSCALE
};

extern VFilterBase *GetVideoFilterFromID(VideoFilter filter);
class HasVideoFilter {
public:
    VFilterBase *vfilter = nullptr;
    VideoFilter filtering = VideoFilter::NONE;

    void initFilter(VideoFilter filter) {
        filtering = filter;
        if (vfilter) { delete vfilter; vfilter = nullptr; }
        vfilter = GetVideoFilterFromID(filter);
        vfilter->initialize();
    }
};

#include "../nes/nes_ppu.hpp"