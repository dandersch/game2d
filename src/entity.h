#pragma once

#include "pch.h"

struct Sprite
{
    SDL_Rect     box;
    SDL_Texture* tex;
    glm::vec2    pivot;
};

enum class EntityFlag
{
    NONE,
    PLAYER_CONTROLLED,
    IS_COLLIDER
};

struct Entity
{
    b32  active; // determines if needs updating
    b32  freed;  // determines if can be replaced with new entity
    u32  flags;
    glm::vec3 position;
    u32  orient;
    u32  renderLayer;

    Sprite sprite;

    //SDL_Rect collider; // TODO box2d
    //EntityState  state;

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
    u32 tileID;

    // P.I.T. contains pos, state, orient, active
    PointInTime* frames = new PointInTime[FPS * LOOPLENGTH + TOLERANCE];

    struct Tile
    {
        u32 tileType;
    };

    SoundBuffer sfx[];
    */
};
