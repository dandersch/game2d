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

        window = SDL_CreateWindow("SDL2 Game", SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight,
                                  SDL_WINDOW_RESIZABLE);
        SDL_ERROR(window);

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        SDL_ERROR(renderer);
    };

    // TODO take in camera pos
    void render(const Sprite& spr, glm::vec3 position)
    {
        SDL_Rect dst = {(int) position.x, (int) position.y,
                        spr.box.w, spr.box.h};
        SDL_RenderCopy(renderer, spr.tex, &spr.box, &dst);
    };

    SDL_Renderer* renderer;
    SDL_Window* window;
};
