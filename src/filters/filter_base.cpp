#include "filters.hpp"

VFilterBase *GetVideoFilterFromID(VideoFilter filter) {
    switch (filter) {
        case VideoFilter::NONE: return new DefaultFilter();
        case VideoFilter::NTSC: return new NTSCFilter();
        case VideoFilter::CHROMA: return new ChromaFilter();
        case VideoFilter::GRAYSCALE: return new GrayScaleFilter();
    }
    return nullptr;
}