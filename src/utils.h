#pragma once

#include "pch.h"

#include "SDL_rect.h"

// TODO don't call sdl functions

inline b32 point_in_rect(point_t point, rect_t rect)
{
    return SDL_PointInRect((SDL_Point*) &point, (SDL_Rect*) &rect);
}

inline b32 rect_empty(rect_t rect) { return SDL_RectEmpty((SDL_Rect*) &rect); }

inline rect_t rect_intersection(rect_t rect1, rect_t rect2)
{
    rect_t result;
    SDL_IntersectRect((SDL_Rect*) &rect1, (SDL_Rect*) &rect2, (SDL_Rect*) &result);
    return result;
}

// NOTE built-in sdl function seems faster
inline b32 rect_intersects(rect_t rect1, rect_t rect2)
{
    return rect1.x + rect1.w >= rect2.x &&
           rect2.x + rect2.w >= rect1.x &&
           rect1.y + rect1.h >= rect2.y &&
           rect2.y + rect2.h >= rect1.y;

    //return SDL_HasIntersection((SDL_Rect*) &rect1, (SDL_Rect*) &rect2);
}
