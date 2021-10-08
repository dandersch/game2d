#pragma once

#include "animation.h"
#include "rewind.h"

struct sprite_t
{
    rect_t box;
    void*  tex;                  // SDL_Texture*  tex;
    v2f    pivot = {0.5f, 0.5f}; //glm::vec2   pivot = {0.5f, 0.5f};
    u32    flip  = 0;            // SDL_RendererFlip flip = SDL_FLIP_NONE;
};

// TODO use regular enum...
enum class EntityFlag : u32
{
    NONE               = 0,
    PLAYER_CONTROLLED  = (1 << 0),
    CMD_CONTROLLED     = (1 << 1),
    IS_ANIMATED        = (1 << 2),
    IS_TILE            = (1 << 3),
    IS_COLLIDER        = (1 << 4),
    PICKUP_BOX         = (1 << 5),
    ATTACK_BOX         = (1 << 6),
    IS_ENEMY           = (1 << 7),
    IS_ITEM            = (1 << 8),
    IS_REWINDABLE      = (1 << 9),
};

enum EntityState
{
    STATE_IDLE, STATE_MOVE, STATE_ATTACK,
    STATE_COUNT
};

enum EntityOrientation
{
    ORIENT_DOWN, ORIENT_RIGHT, ORIENT_UP, ORIENT_LEFT,
    ORIENT_COUNT
};

// TODO default values
struct Entity
{
    // sets position in regards of entities pivot point (center by default)
    void setPivPos(v3f pos)
    {
        position = {pos.x - (sprite.box.w * sprite.pivot.x),
                    pos.y - (sprite.box.h * sprite.pivot.y), 0};
    }

    // get position without pivot (i.e. topleft of spritebox)
    v3f getUnpivPos()
    {
        return {position.x + (sprite.box.w * sprite.pivot.x),
                position.y + (sprite.box.h * sprite.pivot.y), 0};
    }

    // collider itself is relative to entity
    rect_t getColliderInWorld()
    {
        return {(i32) (position.x + collider.x),
                (i32) (position.y + collider.y),
                collider.w, collider.h};
    }

    b32  active; // determines if needs updating
    b32  freed = true;  // determines if can be replaced with new entity
    u32  flags;
    v3f  position; // NOTE maybe make private or similar bc we want pos to be set with setPivPos
    f32  scale = 1.0f;
    u32  orient;
    u32  renderLayer;

    sprite_t sprite;

    rect_t collider; // TODO box2d?
    //MyTile tile;

    u32  state;

    //AnimationClip clips[STATE_COUNT * ORIENT_COUNT] = {};
    AnimationClip clips[1000] = {};
    Animator anim = {nullptr, 0, 0};
    u32 clip_count = 0;
    //Animation anim; // TODO use an index that accesses into anims instead
    //Animation anims[STATE_COUNT * ORIENT_COUNT];

    v3f movement; // desired movement for this frame, used by collision.h

    // TODO fill up with nullcommands at start?
    //Command* cmds[MAX_CMD_COUNT]; // command array for replay
    Command* cmds = nullptr; // command array for replay
    //u32 cmdIdx = 0;

    // contains pos, state, orient, active
    PointInTime* frames = nullptr;

    Entity* owner = nullptr; // for items
    Entity* item  = nullptr;

    /*
    u32 charID; // TODO read in from .tmx
    SoundBuffer sfx[];
    */
};

// TILES ///////////////////////////////////////////////////////////////////////////////////////////
enum class TileType
{
    GRASS,
    DIRT
};

struct Tile
{
    bool     collidable  = false;
    //TileType type;
    u32      tileID      = 0;
    v3f      position    = {0,0,0};
    sprite_t sprite      = {{0,0,0,0}, nullptr, {0,0}, 0 /*SDL_FLIP_NONE*/};
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
#define MAX_ENTITIES_WO_TEMP  1000

// TODO all allocations & deallocations of entities should go through here
// NOTE maybe use implementation of pool w/ free lists:
// https://www.gingerbill.org/article/2019/02/16/memory-allocation-strategies-004/
namespace EntityMgr
{
    // TODO std::move?
    bool copyEntity(const Entity ent);
    Entity* getArray();
    void freeTemporaryStorage();
    bool copyTempEntity(const Entity ent);

    // tile functions
    bool createTile(const Tile tile);
    Tile* getTiles();
    u32 getTileCount();
};
