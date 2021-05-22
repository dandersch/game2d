#include "player.h"

#include "collision.h"
#include "gamelayer.h"
#include "entity.h"

#include "command.h"
#include "input.h"

const f32 Player::playerSpeed = 150.f;

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

    // use inputhandler
    u32 input = Input::actionState;
    if (input & ACTION_MOVE_UP) movement = glm::vec3( 0,-1,0) * playerSpeed * dt;
    if (input & ACTION_MOVE_LEFT) movement = glm::vec3(-1, 0,0) * playerSpeed * dt;
    if (input & ACTION_MOVE_DOWN) movement = glm::vec3( 0, 1,0) * playerSpeed * dt;
    if (input & ACTION_MOVE_RIGHT) movement = glm::vec3( 1, 0,0) * playerSpeed * dt;

    // use commands instead of calling tryMove directly
    auto moveCmd = new MoveCommand(movement);
    CommandProcessor::record(ent, moveCmd);
    //tryMove(movement, ent);
}

void Player::tryMove(glm::vec3 movement, Entity& ent)
{
    Orientation newOrient = (Orientation) ent.orient;
    if      (movement.y < 0.0f) newOrient = ORIENT_UP;
    else if (movement.y > 0.0f) newOrient = ORIENT_DOWN;
    else if (movement.x > 0.0f) newOrient = ORIENT_RIGHT;
    else if (movement.x < 0.0f) newOrient = ORIENT_LEFT;
    u32 newState = ent.state;
    if (movement != glm::vec3{0,0,0})  newState = STATE_MOVE;
    else newState = STATE_IDLE;

    //ChangeAnimationState(newAnim, newOrient);
    ent.state  = newState;
    if (ent.orient != newOrient)
    {
        ent.orient = newOrient;
        ent.anim   = ent.anims[ent.orient]; // TODO support states
    }

    ent.movement = movement;
}
