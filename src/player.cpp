#include "player.h"

#include "collision.h"
#include "entity.h"
#include "input.h"
#include "platform.h"

static const f32 playerSpeed = 150.f;

#include "memory.h"
extern game_state_t* state;

v3f getDirectionFrom(u32 orient)
{
    v3f dir = {0,0,0};
    switch (orient) {
        case ORIENT_UP:    { dir = {  0,-16,0}; } break;
        case ORIENT_DOWN:  { dir = {  0, 32,0}; } break;
        case ORIENT_LEFT:  { dir = {-16, 16,0}; } break;
        case ORIENT_RIGHT: { dir = { 16, 16,0}; } break;
    }

    return dir;
}

// TESTING SDL TIMER CALLBACKS /////////////////////////////
bool isPickingUp = false;
u32 callback(u32 interval, void* param )
{
    printf( "PICKUP DONE AFTER: %s\n", (char*)param );
    isPickingUp = false;
    return 0;
}

void player_update(f32 dt, Entity &ent)
{
    // TODO block input while picking up, attacking etc.
    if (isPickingUp) return;

    // TODO we shouldnt move entities in here, just record the input
    Command::Type cmdtype = Command::MOVE;
    v3f movement = {0,0,0};

    // use inputhandler
    u32 input = state->actionState;
    if (input & ACTION_MOVE_UP)    movement = { 0,-1 * playerSpeed * dt, 0};
    if (input & ACTION_MOVE_LEFT)  movement = {-1 * playerSpeed * dt, 0, 0};
    if (input & ACTION_MOVE_DOWN)  movement = { 0, 1 * playerSpeed * dt, 0};
    if (input & ACTION_MOVE_RIGHT) movement = { 1 * playerSpeed * dt, 0, 0};

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

    // attach item to player TODO doesn't run when replaying cmds
    if (ent.item != nullptr)
        ent.item->setPivPos({ent.position.x + 8, ent.position.y + (-8), ent.position.z + 0});

    // use commands instead of calling tryMove directly
    CommandProcessor::record(ent, {cmdtype, movement});
}

void player_try_move(v3f movement, Entity& ent)
{
    Orientation newOrient = (Orientation) ent.orient;
    if      (movement.y < 0.0f) newOrient = ORIENT_UP;
    else if (movement.y > 0.0f) newOrient = ORIENT_DOWN;
    else if (movement.x > 0.0f) newOrient = ORIENT_RIGHT;
    else if (movement.x < 0.0f) newOrient = ORIENT_LEFT;
    u32 newState = ent.state;
    if (movement.x != 0 && movement.y != 0 && movement.z != 0)  newState = STATE_MOVE;
    else newState = STATE_IDLE;

    //ChangeAnimationState(newAnim, newOrient);
    ent.state  = newState;
    if (ent.orient != newOrient)
    {
        ent.orient = newOrient;
        //ent.anim   = ent.anims[ent.orient]; // TODO support states
    }

    ent.movement = movement;
}

// #include <SDL_timer.h>  // TODO remove
// #include <SDL_events.h> // TODO remove
void player_try_pickup(v3f direction, Entity& ent)
{
    isPickingUp = true;
    // SDL_TimerID timerID = SDL_AddTimer(1 * 1000, callback, (void*) "1 second!");

    v3f pickupPos = {ent.position.x + direction.x,
                     ent.position.y + direction.y,
                     ent.position.z + direction.z};

    // put already held item down
    if (ent.item)
    {
        //pickupPos = ent.position + direction;
        ent.item->setPivPos(pickupPos); // TODO
        ent.item->flags |= (u32) EntityFlag::IS_COLLIDER;
        ent.item = nullptr;
        return;
    }

    // create a collision box at playerpos + direction
    // TODO try to check for collisions directly
    // TODO add flag to not render this
    Entity pickupBox = {0};
    pickupBox.active = true;
    pickupBox.owner  = &ent;
    pickupBox.freed  = false;
    pickupBox.flags |= (u32) EntityFlag::IS_COLLIDER;
    pickupBox.flags |= (u32) EntityFlag::PICKUP_BOX;
    pickupBox.sprite.pivot = {0.5f, 0.5f};
    pickupBox.setPivPos({pickupPos});
    pickupBox.collider = { .x = 0, //(int) pickupPos.x ,
                           .y = 0, //(int) pickupPos.y,
                           .w = 16, .h = 16};
    pickupBox.renderLayer = 1;
    pickupBox.movement = {0,0,0};
    EntityMgr::copyTempEntity(pickupBox);
}

void player_try_attack(v3f direction, Entity& ent)
{
    // create a collision box at playerpos + direction
    v3f attack_pos = {ent.position.x + direction.x,
                     ent.position.y + direction.y,
                     ent.position.z + direction.z};
    Entity attackBox = {0};
    attackBox.active = true;
    attackBox.freed  = false;
    attackBox.flags |= (u32) EntityFlag::IS_COLLIDER;
    attackBox.flags |= (u32) EntityFlag::ATTACK_BOX;
    attackBox.collider = { .x = (int) attack_pos.x ,
                           .y = (int) attack_pos.y,
                           .w = 16, .h = 16};
    EntityMgr::copyTempEntity(attackBox);
}
