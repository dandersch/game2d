#pragma once

#include "pch.h"
#include "entity.h"
#include "layer.h"

const f32 playerSpeed = 1.f;

class Player
{
public:
    static void handleEvent(const Event& e,  Entity& ent)
    {
        SDL_Event evn = e.evn;

        switch (evn.type) {
        case SDL_KEYDOWN:
            switch (evn.key.keysym.sym)
            {
            case SDLK_w:
                ent.position += glm::vec3( 0,-1,0) * playerSpeed;
                break;
            case SDLK_a:
                ent.position += glm::vec3(-1, 0,0) * playerSpeed;
                break;
            case SDLK_s:
                ent.position += glm::vec3( 0, 1,0) * playerSpeed;
                break;
            case SDLK_d:
                ent.position += glm::vec3( 1, 0,0) * playerSpeed;
                break;
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            ent.position = glm::vec3(evn.button.x, evn.button.y, 0);
            break;
        }
    }
};
