#pragma once

#include "entity.h"
#include "tile.h"

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
