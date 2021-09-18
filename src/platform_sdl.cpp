#include "platform_sdl.h"
#include "SDL_render.h"
#include "pch.h"
#include "platform.h"

/*
struct platform_window_t
{
    SDL_Window*   handle;
    SDL_Renderer* renderer;
};
*/

#include "entity.h" // needed for sprite struct, TODO remove

platform_window_t* platform_window_open(const char* title, u32 screen_width, u32 screen_height)
{
    if (SDL_Init(SDL_INIT_TIMER
                 | SDL_INIT_AUDIO
                 | SDL_INIT_VIDEO
                 // | SDL_INIT_JOYSTICK
                 // | SDL_INIT_HAPTIC
                 // | SDL_INIT_GAMECONTROLLER
                 // | SDL_INIT_EVENTS
                 // | SDL_INIT_SENSOR
                 // | SDL_INIT_NOPARACHUTE
                 // | SDL_INIT_EVERYTHING
                 ) != 0)
    {
        printf("SDL init failed: %s\n", SDL_GetError());
    }

    platform_window_t* window = (platform_window_t*) malloc(sizeof(platform_window_t));

    window->handle = SDL_CreateWindow(title,
                                               SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                               screen_width, screen_height, 0);
    SDL_ERROR(window->handle);

    window->renderer = SDL_CreateRenderer(window->handle, -1,
                                          SDL_RENDERER_ACCELERATED
                                          //| SDL_RENDERER_PRESENTVSYNC
                                          );
    SDL_ERROR(window->renderer);

    //SDL_RenderSetLogicalSize(rw->renderer, 640, 480);
    //SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    //printf("%s\n", SDL_GetHint("SDL_HINT_RENDER_SCALE_QUALITY"));

    SDL_RenderSetScale(window->renderer, 1.f, 1.f);

    return window;
}

void platform_render(platform_window_t* window, const sprite_t& spr,
                     v3f position, f32 scale, u32 flip_type)
{
   SDL_Rect dst = {(int) position.x, (int) position.y,
                   (i32) (scale * spr.box.w), (i32) (scale * spr.box.h)};

   // NOTE: flipping seems expensive, maybe just store flipped sprites in
   // the spritesheet & add dedicated animations for those
   SDL_RenderCopyEx(window->renderer, (SDL_Texture*) spr.tex, (SDL_Rect*) &spr.box,
                    &dst, 0, NULL, (SDL_RendererFlip) flip_type);
   //SDL_RenderCopy(renderer, spr.tex, &spr.box, &dst);

}

void platform_render_clear(platform_window_t* window)
{
    SDL_RenderClear(window->renderer);
}

void platform_render_present(platform_window_t* window)
{
    SDL_RenderPresent(window->renderer);
}

void platform_render_set_draw_color(platform_window_t* window, u8 r, u8 g, u8 b, u8 a)
{
    SDL_SetRenderDrawColor(window->renderer, r, g, b, a);
}

void platform_debug_draw(platform_window_t* window, const Entity& e, v3f pos)
{
    SDL_Rect dst = {(int) pos.x + e.collider.x, (int) pos.y + e.collider.y,
                    (i32) (e.scale * e.collider.w), (i32) (e.scale * e.collider.h)};

    // don't draw 'empty' colliders (otherwise it will draw points & lines)
    if (!SDL_RectEmpty(&dst)) // if (!(dst.h <= 0.f && dst.w <= 0.f))
        SDL_RenderDrawRect(window->renderer, &dst);

    //SDL_RenderDrawLine(SDL_Renderer *renderer, int x1, int y1, int x2, int y2)

    // TODO isn't where it's expected
    // Draw pivot point
    SDL_RenderDrawPointF(window->renderer, pos.x, pos.y);
}

void platform_debug_draw_rect(platform_window_t* window, rect_t* dst)
{
    SDL_RenderDrawRect(window->renderer, (SDL_Rect*) dst);
}
