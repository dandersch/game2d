#pragma once

#include "pch.h"
#include "entity.h"
#include "tile.h"

#define MAX_ENTITIES          1100
#define MAX_TILES             15000
#define MAX_ENTITIES_WO_TEMP  1000

// TODO all allocations & deallocations of entities should go through here
// NOTE maybe use implementation of pool w/ free lists:
// https://www.gingerbill.org/article/2019/02/16/memory-allocation-strategies-004/
class EntityMgr
{
public:
    // TODO std::move?
    static bool copyEntity(const Entity ent);
    static Entity* getArray();
    static void freeTemporaryStorage();
    static bool copyTempEntity(const Entity ent);

    // tile functions
    static bool createTile(const Tile tile);
    static Tile* getTiles();
    static u32 getTileCount();
};
