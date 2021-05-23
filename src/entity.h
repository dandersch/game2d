#pragma once

#include "pch.h"
#include "animation.h"
#include "rewind.h"
#include "tile.h"
#include "command.h"

struct Sprite
{
    SDL_Rect         box;
    SDL_Texture*     tex;
    glm::vec2        pivot = {0.5f, 0.5f};
    SDL_RendererFlip flip = SDL_FLIP_NONE;
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
    IS_ENEMY           = (1 << 5),
    IS_REWINDABLE      = (1 << 6),
};

enum EntityState
{
    STATE_IDLE, STATE_MOVE, STATE_ATTACK,
    STATE_COUNT
};

enum Orientation
{
    ORIENT_DOWN, ORIENT_RIGHT, ORIENT_UP, ORIENT_LEFT,
    ORIENT_COUNT
};

struct Entity
{
    // sets position in regards of entities pivot point (center by default)
    void setPivPos(glm::vec3 pos)
    {
        position = {pos.x - (sprite.box.w * sprite.pivot.x),
                    pos.y - (sprite.box.h * sprite.pivot.y), 0};
    }

    // get position without pivot (i.e. topleft of spritebox)
    glm::vec3 getUnpivPos()
    {
        return {position.x + (sprite.box.w * sprite.pivot.x),
                position.y + (sprite.box.h * sprite.pivot.y), 0};
    }

    b32  active; // determines if needs updating
    b32  freed = true;  // determines if can be replaced with new entity
    u32  flags;
    glm::vec3 position; // NOTE maybe make private or similar bc we want pos to
                        // be set with setPivPos
    f32  scale = 1.0f;
    u32  orient;
    u32  renderLayer;

    Sprite sprite;
    Animation anim; // TODO use an index that accesses into anims instead

    SDL_Rect collider; // TODO box2d?
    MyTile tile;

    u32  state;

    Animation anims[STATE_COUNT * ORIENT_COUNT];
    glm::vec3 movement; // desired movement for this frame, used by collision.h

    // TODO fill up with nullcommands at start?
    //Command* cmds[MAX_CMD_COUNT]; // command array for replay
    Command* cmds = nullptr; // command array for replay
    //u32 cmdIdx = 0;

    // contains pos, state, orient, active
    PointInTime* frames = nullptr;

    /*
    u32 charID; // TODO read in from .tmx
    SoundBuffer sfx[];
    */
};
