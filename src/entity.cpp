#include "entity.h"

bool EntityMgr::copyEntity(const Entity ent, game_state_t* state)
{
    for (u32 i = 0; i < MAX_ENTITIES; i++)
    {
        if (state->ents[i].freed)
        {
            state->ents[i] = ent;
            return true;
        }
    }

    return false;
}

bool EntityMgr::createTile(const Tile tile, game_state_t* state)
{
    ASSERT(state->tile_count < MAX_TILES);

    state->tiles[state->tile_count++] = tile;
    return true;
}
