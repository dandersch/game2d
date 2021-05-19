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
};
