#include "entitymgr.h"

#include "tile.h"

#include "memory.h"
extern game_state_t* state;

// TODO std::move?
bool EntityMgr::copyEntity(const Entity ent)
{
    for (u32 i = 0; i < MAX_ENTITIES_WO_TEMP; i++)
    {
        if (state->ents[i].freed)
        {
            state->ents[i] = ent;
            if (ent.flags & (u32) EntityFlag::IS_ANIMATED)
            {
                // TODO should be done elsewhere
                state->ents[i].anim.current_clip = &state->ents[i].clips[0];
            }
            return true;
        }
    }

    return false;
}

#include <cstring>
void EntityMgr::freeTemporaryStorage()
{
    memset(&state->ents[MAX_ENTITIES_WO_TEMP],
           0, MAX_ENTITIES - MAX_ENTITIES_WO_TEMP);
    state->temp_count = 0;
}

u32 EntityMgr::getTileCount()
{
    return state->tile_count;
}

Tile* EntityMgr::getTiles()
{
    return state->tiles;
}

bool EntityMgr::createTile(const Tile tile)
{
    ASSERT(tile_count < MAX_TILES);

    state->tiles[state->tile_count++] = tile;
    return true;
}

bool EntityMgr::copyTempEntity(const Entity ent)
{
    ASSERT(temp_count < 100);
    state->ents[MAX_ENTITIES_WO_TEMP + state->temp_count] = ent;
    state->temp_count++;
    return true; // TODO
}

Entity* EntityMgr::getArray()
{
    return &state->ents[0];
}
