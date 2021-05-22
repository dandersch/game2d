#pragma once

#include "SDL_rect.h"
#include "SDL_render.h"
#include "pch.h"
#include "entity.h"

class RenderWindow
{
public:
    RenderWindow(u32 screenWidth, u32 screenHeight)
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
            printf("SDL init failed: %s\n", SDL_GetError());


        window = SDL_CreateWindow("SDL2 Game",
                                  SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED,
                                  screenWidth, screenHeight,
                                  SDL_WINDOW_RESIZABLE);
        SDL_ERROR(window);

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED
                                                //| SDL_RENDERER_PRESENTVSYNC
                                     );
        SDL_ERROR(renderer);

        //SDL_RenderSetLogicalSize(rw->renderer, 640, 480);
        //SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
        //printf("%s\n", SDL_GetHint("SDL_HINT_RENDER_SCALE_QUALITY"));
    };

    // TODO take in camera pos
    void render(const Sprite& spr, glm::vec3 position, f32 scale = 1.0f,
                SDL_RendererFlip flip = SDL_FLIP_NONE)
    {
        SDL_Rect dst = {(int) position.x, (int) position.y,
                        (i32) (scale * spr.box.w), (i32) (scale * spr.box.h)};

        // NOTE: flipping seems expensive, maybe just store flipped sprites in
        // the spritesheet & add dedicated animations for those
        SDL_RenderCopyEx(renderer, spr.tex, &spr.box, &dst, 0, NULL, flip);
        //SDL_RenderCopy(renderer, spr.tex, &spr.box, &dst);
    };

    // draw colliders, pivot points, etc.
    void debugDraw(const Entity& e, glm::vec3 pos)
    {
        SDL_Rect dst = {(int) pos.x + e.collider.x, (int) pos.y + e.collider.y,
                        (i32) (e.scale * e.collider.w),
                        (i32) (e.scale * e.collider.h)};

        // don't draw 'empty' colliders (otherwise it will draw points & lines)
        if (!SDL_RectEmpty(&dst)) // if (!(dst.h <= 0.f && dst.w <= 0.f))
            SDL_RenderDrawRect(rw->renderer, &dst);

        //SDL_RenderDrawLine(SDL_Renderer *renderer, int x1, int y1, int x2, int y2)

        // TODO isn't where it's expected
        // Draw pivot point
        //SDL_RenderDrawPointF(rw->renderer, pos.x, pos.y);
    };

    SDL_Renderer* renderer;
    SDL_Window* window;
};
