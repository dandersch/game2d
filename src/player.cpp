#include "player.h"

#include "collision.h"
#include "gamelayer.h"
#include "entity.h"

void Player::handleEvent(const Event& e,  Entity& ent, const Camera& cam)
{
    SDL_Event evn = e.evn;

    switch (evn.type) {
    case SDL_MOUSEBUTTONDOWN:
        auto click = cam.screenToWorld({evn.button.x, evn.button.y, 0});
        ent.setPivPos(glm::vec3(click.x, click.y, 0));
        break;
    }
}

void Player::update(f32 dt, Entity &ent)
{
    // TODO we shouldnt move entities in here, just record the input
    glm::vec3 movement = {0,0,0};

    const Uint8* keystate = SDL_GetKeyboardState(NULL);
    if (keystate[SDL_SCANCODE_W])
        movement = glm::vec3( 0,-1,0) * playerSpeed * dt;
    if (keystate[SDL_SCANCODE_A])
        movement = glm::vec3(-1, 0,0) * playerSpeed * dt;
    if (keystate[SDL_SCANCODE_S])
        movement = glm::vec3( 0, 1,0) * playerSpeed * dt;
    if (keystate[SDL_SCANCODE_D])
        movement = glm::vec3( 1, 0,0) * playerSpeed * dt;

    tryMove(movement, ent);
}

void Player::tryMove(glm::vec3 movement, Entity& ent)
{
    Orientation newOrient = (Orientation) ent.orient;
    if      (movement.y < 0.0f) newOrient = Orientation::UP;
    else if (movement.y > 0.0f) newOrient = Orientation::DOWN;
    else if (movement.x > 0.0f) newOrient = Orientation::RIGHT;
    else if (movement.x < 0.0f) newOrient = Orientation::LEFT;
    EntityState newState = (EntityState) ent.state;
    if (movement != glm::vec3{0,0,0})  newState = EntityState::MOVE;
    else newState = EntityState::IDLE;

    //ChangeAnimationState(newAnim, newOrient);
    ent.state  = newState;
    if (ent.orient != (u32) newOrient)
    {
        ent.orient = (u32) newOrient;
        ent.anim   = ent.anims[ent.orient]; // TODO support states
    }

    ent.movement = movement;
}
