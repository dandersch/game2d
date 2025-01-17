#pragma once
#include "animation.h"
#include "rewind.h"

struct sprite_t
{
    rect_t      box;
    texture_t*  tex;
    v2f         pivot = {0.5f, 0.5f};
    //u32    flip  = 0;            // == SDL_RendererFlip flip = SDL_FLIP_NONE; NOTE unused
};

enum entity_flag_e
{
    ENT_FLAG_NONE               = 0,
    ENT_FLAG_PLAYER_CONTROLLED  = (1 << 0),
    ENT_FLAG_CMD_CONTROLLED     = (1 << 1),
    ENT_FLAG_IS_ITEM            = (1 << 2),
    ENT_FLAG_IS_TILE            = (1 << 3),
    ENT_FLAG_IS_COLLIDER        = (1 << 4),
    ENT_FLAG_PICKUP_BOX         = (1 << 5),
    ENT_FLAG_ATTACK_BOX         = (1 << 6),
    ENT_FLAG_IS_ENEMY           = (1 << 7), // max for collision callbacks array
    ENT_FLAG_IS_ANIMATED        = (1 << 8),
    ENT_FLAG_IS_REWINDABLE      = (1 << 9),
};

enum entity_state_e
{
    //ENT_STATE_INVALID,
    ENT_STATE_IDLE,
    ENT_STATE_MOVE,
    ENT_STATE_ATTACK,
    ENT_STATE_HOLD,
    ENT_STATE_COUNT
};

enum entity_orientation_e
{
    //ENT_ORIENT_INVALID,
    ENT_ORIENT_UP,
    ENT_ORIENT_DOWN,
    ENT_ORIENT_RIGHT,
    ENT_ORIENT_LEFT,
    ENT_ORIENT_COUNT
};

// TODO default values
struct Entity
{
    // u32 charID; // TODO read in from .tmx
    b32      active;               // determines if needs updating NOTE ZII violation ?
    b32      freed        = true;  // determines if can be replaced with new entity NOTE ZII violation
    u32      flags; // TODO try out using just bools for this
    u32      state;
    u32      orient;
    v3f      position;             // NOTE maybe make private or similar bc we want pos to be set with setPivPos
    f32      scale        = 1.0f;  // NOTE violation against ZII
    sprite_t sprite;
    rect_t   collider;             // TODO box2d?
    v3f      movement;             // desired movement for this frame, used by physics

    char type[20]; // TODO

    f32 anim_timer = 0.0f;
    animation_t anims[ENT_STATE_COUNT * ENT_ORIENT_COUNT] = {};

    // TODO fill up with nullcommands at start?
    //Command* cmds[MAX_CMD_COUNT]; // command array for replay
    Command* cmds = nullptr; // command array for replay
    PointInTime* frames = nullptr;   // contains pos, state, orient, active

    // ITEMS
    Entity* owner = nullptr; // who is holding this entity
    Entity* item  = nullptr; // what is this entity holding
    // SoundBuffer sfx[];

    void setPivPos(v3f pos) // sets position in regards of entities pivot point (center by default)
    {
        position = {pos.x - (sprite.box.w * sprite.pivot.x), pos.y - (sprite.box.h * sprite.pivot.y), 0};
    }

    v3f getUnpivPos() // get position without pivot (i.e. topleft of spritebox)
    {
        return {position.x + (sprite.box.w * sprite.pivot.x), position.y + (sprite.box.h * sprite.pivot.y), 0};
    }

    rect_t getColliderInWorld() // collider itself is relative to entity
    {
        return {(i32) (position.x + collider.left), (i32) (position.y + collider.top), collider.w, collider.h};
    }
};

// TILES ///////////////////////////////////////////////////////////////////////////////////////////
struct Tile
{
    bool     collidable  = false;
    u32      tileID      = 0;
    v3f      position    = {0,0,0};
    sprite_t sprite      = {0};
    u32      renderLayer = 0;
    rect_t   collider    = {0,0,0,0};

    void setPivPos(v3f pos)
    {
        position = {pos.x - (sprite.box.w * sprite.pivot.x),
                    pos.y - (sprite.box.h * sprite.pivot.y), 0};
    }
};

// ENTITYMANAGER ///////////////////////////////////////////////////////////////////////////////////
#define MAX_ENTITIES          1100
#define MAX_TILES             15000

// TODO all allocations & deallocations of entities should go through here
// NOTE maybe use implementation of pool w/ free lists:
// https://www.gingerbill.org/article/2019/02/16/memory-allocation-strategies-004/
namespace EntityMgr
{
    bool copyEntity(const Entity ent, game_state_t* state);
    bool createTile(const Tile tile, game_state_t* state);

    // typedef u32 entity_handle_t
    // struct entity_handle_t
    // {
    // }
    //
    // entity_handle_t entity_get_handle();
    // entity_t*       entity_get(entity_handle_t handle);
};
