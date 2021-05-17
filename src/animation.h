#pragma once

#include "pch.h"

#define NUMBER_OF_FRAMES 3

struct Animation
{
    //b32 done = true;
    SDL_Rect frames[NUMBER_OF_FRAMES]; // TODO probably go with std::vector
    f32 length = 1.0f; // time for whole animation
    b32 loop  = false;
    b32 flipV = false; // whole animation is flipped vertically
    b32 flipH = false; // whole animation is flipped horizontally
    //u32 m_framewidth;
    //u32 m_frameheight;
    u32 index = 0;
    f32 time = 0;
};

class Animator
{
public:

    // TODO terrible
    static SDL_Rect animate(f32 dt, Animation& anim)
    {
        if (anim.index >= (NUMBER_OF_FRAMES-1) && !anim.loop)
            return anim.frames[anim.index];

        if (anim.index >= (NUMBER_OF_FRAMES-1) && anim.loop)
        {
            anim.index = 0;
            anim.time = 0.f;
        }

        anim.time += dt;

        f32 timeTillNext = (anim.length/NUMBER_OF_FRAMES) * (anim.index+1);

        if (anim.time > timeTillNext) // TODO hardcoded
        {
            if (anim.index < (NUMBER_OF_FRAMES-1)) anim.index++;
        }

        return anim.frames[anim.index];
    }

};
