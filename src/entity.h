#pragma once

#include "pch.h"
#include "animation.h"
#include "tile.h"

struct Sprite
{
    SDL_Rect         box;
    SDL_Texture*     tex;
    glm::vec2        pivot;
    SDL_RendererFlip flip = SDL_FLIP_NONE;
};

enum class EntityFlag
{
    NONE, PLAYER_CONTROLLED,
    IS_ANIMATED,
    IS_TILE,
    IS_COLLIDER
};

enum class EntityState
{
    IDLE, MOVE, ATTACK,
    COUNT
};

enum class Orientation
{
    DOWN, RIGHT, UP, LEFT,
    COUNT
};

struct Entity
{
    b32  active; // determines if needs updating
    b32  freed = true;  // determines if can be replaced with new entity
    u32  flags;
    glm::vec3 position;
    f32  scale = 1.0f;
    u32  orient;
    u32  renderLayer;

    Sprite sprite;
    Animation anim; // TODO use an index that accesses into anims instead

    MyTile tile;

    SDL_Rect collider; // TODO box2d?
    EntityState  state;

    Animation anims[(u32) EntityState::COUNT * (u32) Orientation::COUNT];

    /*
    Command* cmds = new Command[maxCmdCount]; // command array for replay
    u32 cmdIdx    = 0;

    struct Animation
    {
        std::vector<SDL_Rect> clips;
        f32 speed    = 1.f;
        f32 animTime;
    };
    HashMap[int, Animation*] currentAnim;
    u32 charID;


    // P.I.T. contains pos, state, orient, active
    PointInTime* frames = new PointInTime[FPS * LOOPLENGTH + TOLERANCE];

    SoundBuffer sfx[];
    */
};
