#pragma once

#include "pch.h"

namespace Reset
{
    extern bool isRewinding;
    extern const f32 rewindFactor; // can slow down/speed up rewind
    extern f32 loopTime;

    void update(f32 dt);
}
