#include "player.h"

#include "entity.h"
#include "input.h"
#include "platform.h"

static const f32 playerSpeed = 150.f; // NOTE not compatible with code hotloading

#include "memory.h"
extern game_state_t* state;

v3f getDirectionFrom(u32 orient)
{
    v3f dir = {0,0,0};
    switch (orient) {
        case ENT_ORIENT_UP:    { dir = {  0,-16,0}; } break;
        case ENT_ORIENT_DOWN:  { dir = {  0, 32,0}; } break;
        case ENT_ORIENT_LEFT:  { dir = {-16, 16,0}; } break;
        case ENT_ORIENT_RIGHT: { dir = { 16, 16,0}; } break;
    }

    return dir;
}

// TESTING SDL TIMER CALLBACKS /////////////////////////////
bool isPickingUp = false; // NOTE not compatible with code hotloading
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
    command_record(ent, {cmdtype, movement});
}

void player_try_move(v3f movement, Entity& ent)
{
    entity_orientation_e new_orient = (entity_orientation_e) ent.orient;
    if      (movement.y < 0.0f) new_orient = ENT_ORIENT_UP;
    else if (movement.y > 0.0f) new_orient = ENT_ORIENT_DOWN;
    else if (movement.x > 0.0f) new_orient = ENT_ORIENT_RIGHT;
    else if (movement.x < 0.0f) new_orient = ENT_ORIENT_LEFT;

    u32 new_state = ent.state;
    if (movement.x != 0 && movement.y != 0 && movement.z != 0)  new_state = ENT_STATE_MOVE;
    else new_state = ENT_STATE_IDLE;

    //ChangeAnimationState(newAnim, newOrient);

    ent.state  = new_state;
    if (ent.orient != new_orient)
    {
        ent.orient = new_orient;
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

    v3f pickup_pos = ent.position + direction;

    // put already held item down
    if (ent.item)
    {
        ent.item->setPivPos(pickup_pos); // TODO this isn't working as expected
        ent.item->flags |= ENT_FLAG_IS_COLLIDER;
        ent.item = nullptr;
        isPickingUp = false;
        return;
    }

    // create a collision box at playerpos + direction
    // TODO try to check for collisions directly
    // TODO add flag to not render this
    Entity pickup_box = {0};
    pickup_box.active = true;
    pickup_box.owner  = &ent;
    pickup_box.freed  = false;
    pickup_box.flags |= ENT_FLAG_IS_COLLIDER;
    pickup_box.flags |= ENT_FLAG_PICKUP_BOX;
    pickup_box.sprite.pivot = {0.5f, 0.5f};
    pickup_box.setPivPos({pickup_pos});
    pickup_box.collider = { .x = 0, //(int) pickupPos.x,
                            .y = 0, //(int) pickupPos.y,
                           .w = 16, .h = 16};
    pickup_box.renderLayer = 1;
    pickup_box.movement = {0,0,0};
    EntityMgr::copyTempEntity(pickup_box);
    isPickingUp = false;
}

void player_try_attack(v3f direction, Entity& ent)
{
    // create a collision box at playerpos + direction
    v3f attack_pos = ent.position + direction;
    Entity attack_box = {0};
    attack_box.active = true;
    attack_box.freed  = false;
    attack_box.flags |= ENT_FLAG_IS_COLLIDER;
    attack_box.flags |= ENT_FLAG_ATTACK_BOX;
    attack_box.collider = { .x = (int) attack_pos.x , .y = (int) attack_pos.y, .w = 16, .h = 16};
    attack_box.renderLayer = 1;
    //attack_box.setPivPos({attack_pos});
    attack_box.movement = {0,0,0};
    EntityMgr::copyTempEntity(attack_box);
}
