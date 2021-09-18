#pragma once

#include "pch.h"

#include "entity.h"

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
