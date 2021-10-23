#pragma once

inline
b32 utils_point_in_rect(v2i point, rect_t rect)
{
    return (rect.x + rect.w >= point.x &&
                     rect.x <= point.x &&
            rect.y + rect.h >= point.y &&
                     rect.y <= point.y);
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
    return rect1.x + rect1.w >= rect2.x &&
           rect2.x + rect2.w >= rect1.x &&
           rect1.y + rect1.h >= rect2.y &&
           rect2.y + rect2.h >= rect1.y;

    //return SDL_HasIntersection((SDL_Rect*) &rect1, (SDL_Rect*) &rect2);
}
