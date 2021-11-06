#pragma once

struct animation_t
{
    u32 count;         // how many sprites are in the animation (NOTE starts at 0)
    i32 start_pos_x;   // where the sprite animation starts on x-axis
    i32 start_pos_y;   // ... on y-axis
    i32 delta_x;       // dist. to next sprite on x-axis
};

#define SPRITE_DURATION 0.16f

inline void anim_update(f32* timer, rect_t* sprite_box, animation_t anim, f32 dt, b32 looped = true)
{
    *timer += dt;

    // find the correct frame
    u32 frame = 0;
    while (*timer > ((frame+1) * SPRITE_DURATION))
    {
        frame++;
    }

    // wraparound
    if (frame >= anim.count)
    {
        frame = 0;
        if (looped) *timer = 0; // TODO b32 looped should be inside the animation
    }

    sprite_box->left = anim.start_pos_x + (frame * anim.delta_x);
    sprite_box->top  = anim.start_pos_y;
}
