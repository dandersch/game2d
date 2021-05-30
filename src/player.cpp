#include "player.h"

#include "collision.h"
#include "gamelayer.h"
#include "entity.h"

#include "command.h"
#include "input.h"

static const f32 playerSpeed = 150.f;

void player_handle_event(const Event& e,  Entity& ent, const Camera& cam)
{
    SDL_Event evn = e.sdl;

    switch (evn.type) {
    case SDL_MOUSEBUTTONDOWN:
        //auto click = cam.screenToWorld({evn.button.x, evn.button.y, 0});
        //ent.setPivPos(glm::vec3(click.x, click.y, 0));
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

// TESTING SDL TIMER CALLBACKS /////////////////////////////
bool isPickingUp = false;
Uint32 callback( Uint32 interval, void* param )
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

    // attach item to player TODO doesn't run when replaying cmds
    if (ent.item != nullptr)
        ent.item->setPivPos({ent.position + glm::vec3{8,-8,0}});

// use commands instead of calling tryMove directly
    CommandProcessor::record(ent, {cmdtype, movement});
}

void player_try_move(glm::vec3 movement, Entity& ent)
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

void player_try_pickup(glm::vec3 direction, Entity& ent)
{
    isPickingUp = true;
    SDL_TimerID timerID = SDL_AddTimer( 1 * 1000,
                                        callback, (void*) "1 second!" );

    glm::vec3 pickupPos = ent.position + direction;

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

void player_try_attack(glm::vec3 direction, Entity& ent)
{
    // create a collision box at playerpos + direction
    glm::vec3 attackpos = ent.position + direction;
    Entity attackBox = {0};
    attackBox.active = true;
    attackBox.freed  = false;
    attackBox.flags |= (u32) EntityFlag::IS_COLLIDER;
    attackBox.flags |= (u32) EntityFlag::ATTACK_BOX;
    attackBox.collider = { .x = (int) attackpos.x ,
                           .y = (int) attackpos.y,
                           .w = 16, .h = 16};
    EntityMgr::copyTempEntity(attackBox);
}
