#pragma once

#include "pch.h"

namespace Reset
{
    extern bool isRewinding;
    extern const f32 rewindFactor; // can slow down/speed up rewind
    extern f32 loopTime;
    extern const f32 TIME_FOR_LOOP;

    void update(f32 dt);
}