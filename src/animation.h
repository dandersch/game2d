#pragma once

#include "pch.h"

// TODO animation shouldn't have internal state like time & index
// struct Animation
// {
//     //b32 done = true;
//     std::vector<SDL_Rect> frames;
//     f32 length = 1.0f; // time for whole animation
//     b32 loop  = false;
//     b32 flipV = false; // TODO whole animation is flipped vertically
//     b32 flipH = false; // TODO whole animation is flipped horizontally
//     //u32 m_framewidth;
//     //u32 m_frameheight;
//     u32 index = 0;
//     f32 time = 0;
// };


// TODO support backwards playing animations for rewinding
// class Animator
// {
// public:

//     static SDL_Rect animate(f32 dt, Animation& anim)
//     {
//         // early out
//         if (anim.index >= (anim.frames.size() - 1) && !anim.loop)
//             return anim.frames[anim.index];

//         f32 timeForEach = anim.length / anim.frames.size();

//         anim.time += dt;

//         if (anim.time > (timeForEach * (anim.index+1)))
//         {
//             if (anim.index < (anim.frames.size()-1)) anim.index++;
//             else if (anim.loop)
//             {
//                 anim.index = 0;
//                 anim.time = 0.f;
//             }
//         }

//         return anim.frames[anim.index];
//     }
// };


struct AnimationFrame
{
    SDL_Rect frame;
    f32 duration;
};

struct AnimationClip
{
    std::vector<AnimationFrame> frames = {};
    bool loop = true;
};

struct Animator
{
    AnimationClip* current_clip;
    u32 frame_idx;
    f32 time;

    // ... current_state;
    // ... current_orient;
};

inline u32 get_index_into_clip(Animator anim, AnimationClip clip)
{
    u32 idx = 0;
    f32 clip_sum = 0;
    for (u32 i = 0; i < clip.frames.size(); i++)
    {
        if (anim.time > clip_sum)
            clip_sum += clip.frames.at(i).duration;
        else
            break;
        idx = i;
    }
    printf("clipsum: %f\n", clip_sum);

    return idx;
}

inline SDL_Rect animation_update(Animator* anim,
                                 AnimationClip* clips,
                                 u32 clip_count, f32 dt)
{
    // TODO determine if clip needs to change

    u32 clip_idx = get_index_into_clip(*anim, *anim->current_clip);
    anim->time += dt;
    if (clip_idx >= (anim->current_clip->frames.size()-1))
    {
        anim->time = 0;
    }
    return anim->current_clip->frames.at(clip_idx).frame;
}
