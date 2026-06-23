#pragma once

#include "default.hpp"
#include "ntsc.hpp"
#include "chroma.hpp"
#include "grayscale.hpp"

VFilterBase *GetVideoFilterFromID(VideoFilter filter);
