#include "entitymgr.h"

Entity ents[MAX_ENTITIES] = {0};

// TODO std::move?
bool EntityMgr::copyEntity(const Entity ent)
{
    for (u32 i = 0; i < MAX_ENTITIES; i++)
    {
        if (ents[i].freed)
        {
            ents[i] = ent;
            return true;
        }
    }

    return false;
}

u32 EntityMgr::getFreeSlot()
{
}

Entity* EntityMgr::getArray()
{
    return &ents[0];
}
