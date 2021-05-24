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

glm::vec3 getDirectionFrom(u32 orient)
{
    glm::vec3 dir = {0,0,0};
    switch (orient) {
    case ORIENT_UP:    dir = {  0,-16,0}; break;
    case ORIENT_DOWN:  dir = {  0, 32,0}; break;
    case ORIENT_LEFT:  dir = {-16, 16,0}; break;
    case ORIENT_RIGHT: dir = { 16, 16,0}; break;
    }

    return dir;
}

void Player::update(f32 dt, Entity &ent)
{
    // TODO we shouldnt move entities in here, just record the input
    Command::Type cmdtype = Command::MOVE;
    glm::vec3 movement = {0,0,0};

    // use inputhandler
    u32 input = Input::actionState;
    if (input & ACTION_MOVE_UP) movement = glm::vec3( 0,-1,0) * playerSpeed * dt;
    if (input & ACTION_MOVE_LEFT) movement = glm::vec3(-1, 0,0) * playerSpeed * dt;
    if (input & ACTION_MOVE_DOWN) movement = glm::vec3( 0, 1,0) * playerSpeed * dt;
    if (input & ACTION_MOVE_RIGHT) movement = glm::vec3( 1, 0,0) * playerSpeed * dt;

    if (input & ACTION_PICKUP)
    {
        movement = getDirectionFrom(ent.orient);
        cmdtype  = Command::PICKUP;
    }

    if (input & ACTION_ATTACK)
    {
        movement = getDirectionFrom(ent.orient);
        cmdtype  = Command::ATTACK;
    }
// use commands instead of calling tryMove directly
    CommandProcessor::record(ent, {cmdtype, movement});
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

void Player::tryPickUp(glm::vec3 direction, Entity& ent)
{
    // create a collision box at playerpos + direction
    glm::vec3 pickupPos = ent.position + direction;
    Entity pickupBox;
    pickupBox.active = true;
    pickupBox.freed  = false;
    pickupBox.flags |= (u32) EntityFlag::IS_COLLIDER;
    pickupBox.flags |= (u32) EntityFlag::PICKUP_BOX;
    pickupBox.collider = { .x = (int) pickupPos.x ,
                           .y = (int) pickupPos.y,
                           .w = 16, .h = 16};
    EntityMgr::copyTempEntity(pickupBox);
}

void Player::tryAttack(glm::vec3 direction, Entity& ent)
{
    // create a collision box at playerpos + direction
    glm::vec3 attackpos = ent.position + direction;
    Entity attackBox;
    attackBox.active = true;
    attackBox.freed  = false;
    attackBox.flags |= (u32) EntityFlag::IS_COLLIDER;
    attackBox.flags |= (u32) EntityFlag::ATTACK_BOX;
    attackBox.collider = { .x = (int) attackpos.x ,
                           .y = (int) attackpos.y,
                           .w = 16, .h = 16};
    EntityMgr::copyTempEntity(attackBox);
}
