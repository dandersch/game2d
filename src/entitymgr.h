#pragma once

#include "pch.h"
#include "entity.h"

#define MAX_ENTITIES  15000

// TODO all allocations & deallocations of entities should go through here
// NOTE maybe use implementation of pool w/ free lists:
// https://www.gingerbill.org/article/2019/02/16/memory-allocation-strategies-004/
class EntityMgr
{
public:
    // TODO std::move?
    static bool copyEntity(const Entity ent);
    static Entity* getArray();
    static u32 getFreeSlot();
};
