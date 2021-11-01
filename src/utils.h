#pragma once

inline
b32 utils_point_in_rect(v2i point, rect_t rect)
{
    return (rect.left + rect.w >= point.x &&
                     rect.left <= point.x &&
            rect.top + rect.h >= point.y &&
                     rect.top <= point.y);
    //return SDL_PointInRect((SDL_Point*) &point, (SDL_Rect*) &rect);
}

inline
b32 utils_rect_empty(rect_t rect)
{
    return rect.w <= 0 && rect.h <= 0;
    //return SDL_RectEmpty((SDL_Rect*) &rect);
}

/*
inline rect_t rect_intersection(rect_t rect1, rect_t rect2)
{
    rect_t result;
    SDL_IntersectRect((SDL_Rect*) &rect1, (SDL_Rect*) &rect2, (SDL_Rect*) &result);
    return result;
}
*/

// NOTE built-in sdl function seems faster
inline
b32 utils_rect_intersects(rect_t rect1, rect_t rect2)
{
    return rect1.left + rect1.w >= rect2.left &&
           rect2.left + rect2.w >= rect1.left &&
           rect1.top + rect1.h >= rect2.top &&
           rect2.top + rect2.h >= rect1.top;

    //return SDL_HasIntersection((SDL_Rect*) &rect1, (SDL_Rect*) &rect2);
}
