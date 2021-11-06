#include "entity.h"


bool EntityMgr::copyEntity(const Entity ent, game_state_t* state)
{
    for (u32 i = 0; i < MAX_ENTITIES_WO_TEMP; i++)
    {
        if (state->ents[i].freed)
        {
            state->ents[i] = ent;
            return true;
        }
    }

    return false;
}


void EntityMgr::freeTemporaryStorage(game_state_t* state)
{
    // NOTE we shouldn't have to zero out the memory
    memset(&state->ents[MAX_ENTITIES_WO_TEMP], 0, MAX_ENTITIES - MAX_ENTITIES_WO_TEMP);
    state->temp_count = 0;
}


bool EntityMgr::createTile(const Tile tile, game_state_t* state)
{
    ASSERT(state->tile_count < MAX_TILES);

    state->tiles[state->tile_count++] = tile;
    return true;
}


bool EntityMgr::copyTempEntity(const Entity ent, game_state_t* state)
{
    ASSERT(state->temp_count < 100);
    state->ents[MAX_ENTITIES_WO_TEMP + state->temp_count] = ent;
    state->temp_count++;
    return true; // TODO
}
