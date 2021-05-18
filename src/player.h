#ifndef PLAYER_H_
#define PLAYER_H_

#include "pch.h"
#include "entity.h"

const f32 playerSpeed = 1.f;

class Player
{
public:
    // TODO remove imgui dependency
    static void handleEvent(const SDL_Event& evn, /*ImGuiIO& io,*/ Entity& ent)
    {
        switch (evn.type) {
        case SDL_KEYDOWN:
            switch (evn.key.keysym.sym)
            {
            case SDLK_w:
                ent.position += glm::vec3( 0,-1,0);
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
            //if (io.WantCaptureMouse) return;
            ent.position = glm::vec3(evn.button.x, evn.button.y, 0);
            break;
        }
    }
};

#endif // PLAYER_H_
