#pragma once

#include "pch.h"

enum class TileType
{
    GRASS,
    DIRT
};

struct MyTile
{
    u32      tileID;
    TileType type;
    //glm::vec3 position;
    // u32 renderLayer;
    //Sprite sprite;
    //SDL_Rect collider; // TODO box2d?
};
