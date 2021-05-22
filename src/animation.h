#pragma once

#include "pch.h"

struct Animation
{
    //b32 done = true;
    std::vector<SDL_Rect> frames;
    f32 length = 1.0f; // time for whole animation
    b32 loop  = false;
    b32 flipV = false; // TODO whole animation is flipped vertically
    b32 flipH = false; // TODO whole animation is flipped horizontally
    //u32 m_framewidth;
    //u32 m_frameheight;
    u32 index = 0;
    f32 time = 0;
};


// TODO support backwards playing animations for rewinding
class Animator
{
public:

    static SDL_Rect animate(f32 dt, Animation& anim)
    {
        // early out
        if (anim.index >= (anim.frames.size() - 1) && !anim.loop)
            return anim.frames[anim.index];

        f32 timeForEach = anim.length / anim.frames.size();

        anim.time += dt;

        if (anim.time > (timeForEach * (anim.index+1)))
        {
            if (anim.index < (anim.frames.size()-1)) anim.index++;
            else if (anim.loop)
            {
                anim.index = 0;
                anim.time = 0.f;
            }
        }

        return anim.frames[anim.index];
    }
};
