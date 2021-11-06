#include "player.h"

#include "entity.h"
#include "input.h"
#include "platform.h"

global_var const f32 playerSpeed = 100.f;


v3f direction_from_orient(u32 orient)
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


void player_update(f32 dt, Entity &ent, u32 action_state, u32* cmd_idx)
{
    // TODO we shouldnt move entities in here, just record the input
    Command::Type cmdtype = Command::MOVE;
    v3f movement = {0,0,0};

    /* determine command from action */
    u32 input = action_state;
    if (input & ACTION_MOVE_UP)    movement = { 0,-1 * playerSpeed * dt, 0};
    if (input & ACTION_MOVE_LEFT)  movement = {-1 * playerSpeed * dt, 0, 0};
    if (input & ACTION_MOVE_DOWN)  movement = { 0, 1 * playerSpeed * dt, 0};
    if (input & ACTION_MOVE_RIGHT) movement = { 1 * playerSpeed * dt, 0, 0};

    if (input & ACTION_PICKUP)
    {
        movement = direction_from_orient(ent.orient);
        cmdtype  = Command::PICKUP;
    }

    if (input & ACTION_ATTACK)
    {
        movement = direction_from_orient(ent.orient);
        cmdtype  = Command::ATTACK;
    }

    Command cmd = {cmdtype, movement};
    command_record_or_replay(&ent, &cmd, cmd_idx); // NOTE actually executes the cmd TODO better name
}


global_var bool apply_state_machine = true;
void player_try_move(v3f movement, Entity& ent)
{
    if (apply_state_machine)
    {
        entity_orientation_e new_orient = (entity_orientation_e) ent.orient;
        if      (movement.y < 0.0f) new_orient = ENT_ORIENT_UP;
        else if (movement.y > 0.0f) new_orient = ENT_ORIENT_DOWN;
        else if (movement.x > 0.0f) new_orient = ENT_ORIENT_RIGHT;
        else if (movement.x < 0.0f) new_orient = ENT_ORIENT_LEFT;

        // state machine logic
        u32 new_state = ent.state;
        if (movement.x == 0 && movement.y == 0 && movement.z == 0) new_state = ENT_STATE_IDLE;
        else new_state = ENT_STATE_MOVE;

        ent.state  = new_state;
        if (ent.orient != new_orient)
        {
            ent.orient = new_orient;
        }
    }

    ent.movement = movement;
}


void player_try_pickup(v3f direction, Entity& ent)
{
    // TODO
    printf("pickup not implemented\n");
}


void player_try_attack(v3f direction, Entity& ent)
{
    // TODO
    printf("attack not implemented\n");
}
