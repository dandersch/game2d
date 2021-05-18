#pragma once

#include "pch.h"
#include "entity.h"

class RenderWindow
{
public:
    RenderWindow(u32 screenWidth, u32 screenHeight)
    {
        if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
            printf("SDL init failed: %s\n", SDL_GetError());

        window = SDL_CreateWindow("SDL2 Game",
                                  SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED,
                                  screenWidth, screenHeight,
                                  SDL_WINDOW_RESIZABLE);
        SDL_ERROR(window);

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        SDL_ERROR(renderer);
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
    };

    SDL_Renderer* renderer;
    SDL_Window* window;
};
