#include "entitymgr.h"

#include "tile.h"

static u32 temp_count = 0;
static Entity ents[MAX_ENTITIES] = {0};
static Tile tiles[MAX_TILES] = {0};
static u32 tile_count = 0;

// std::vector<Entity*> toDestroy;

// TODO std::move?
bool EntityMgr::copyEntity(const Entity ent)
{
    for (u32 i = 0; i < MAX_ENTITIES_WO_TEMP; i++)
    {
        if (ents[i].freed)
        {
            ents[i] = ent;
            return true;
        }
    }

    return false;
}

void EntityMgr::freeTemporaryStorage()
{
    memset(&ents[MAX_ENTITIES_WO_TEMP],
           0, MAX_ENTITIES - MAX_ENTITIES_WO_TEMP);
    temp_count = 0;
}

u32 EntityMgr::getTileCount()
{
    return tile_count;
}

Tile* EntityMgr::getTiles()
{
    return tiles;
}

bool EntityMgr::createTile(const Tile tile)
{
    ASSERT(tile_count < MAX_TILES);

    tiles[tile_count++] = tile;
    return true;
}

bool EntityMgr::copyTempEntity(const Entity ent)
{
    ASSERT(temp_count < 100);
    ents[MAX_ENTITIES_WO_TEMP + temp_count] = ent;
    temp_count++;
    return true; // TODO
}

Entity* EntityMgr::getArray()
{
    return &ents[0];
}
