#pragma once

#include "pch.h"


struct Entity;
namespace Collision {

bool AABB(const SDL_Rect& recA, const SDL_Rect& recB);

// TODO for all entities collect their desired movement in one update loop,
// then have one loop for collision detection with that movement vec

// map for callback functions because we access into it with a bitmask, e.g.
// when enemy & character collide, we do
// callbacks[TYPE_ENEMY & TYPECHARACTER](e1, e2)
// and maybe also pass in a struct collisionInfo

bool checkCollision(Entity& e1, Entity& e2);

} // namespace Collision
