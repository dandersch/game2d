#include "collision.h"
#include "entity.h"
#include "tile.h"
#include "utils.h"

static std::map<u32, std::function<void(Entity* e1, Entity* e2)>> callbacks = {
{ (u32) EntityFlag::IS_ENEMY | (u32) EntityFlag::IS_TILE,
  [](Entity* e1, Entity* e2)
  {
      printf("Yes\n");
      // do something
  }},
{ (u32) EntityFlag::PICKUP_BOX | (u32) EntityFlag::IS_ITEM,
  [](Entity* e1, Entity* e2)
  {
      printf("ITEM PICKUP\n");
      e1->owner->item  = e2;
      e2->flags ^= (u32) EntityFlag::IS_COLLIDER;
  }}
};

b32 Collision::AABB(const rect_t& recA, const rect_t& recB)
{
    if (rect_intersects(recA, recB))
        return true;

    return false;
}

void collisionCallback(Entity* e1, Entity* e2, u32 flagCombination)
{
    // TODO use callbacks like this:
    auto cb = callbacks.find(flagCombination);
    if (cb != callbacks.end())
    {
        cb->second(e1, e2);
    }
}

b32 Collision::checkCollisionWithTiles(Entity& e1, Tile& t1)
{
    // TODO use unpivoted pos or calculate pos back to unpivoted here
    // collider of e1 after moving
    rect_t a = {(i32) (e1.position.x + e1.movement.x) + e1.collider.x,
                (i32) (e1.position.y + e1.movement.y) + e1.collider.y,
                e1.collider.w, e1.collider.h};

    // collider of tile
    rect_t b = {(i32) t1.position.x + t1.collider.x,
                (i32) t1.position.y + t1.collider.y,
                t1.collider.w, t1.collider.h};


    b32 collided = Collision::AABB(a, b);
    if (collided)
    {
        e1.movement = {0,0,0};

        // check flags here if needed
    }

    return collided;
}

b32 Collision::checkCollision(Entity& e1, Entity& e2)
{
    // TODO use unpivoted pos or calculate pos back to unpivoted here
    // collider of e1 after moving
    rect_t a = {(i32) (e1.position.x + e1.movement.x) + e1.collider.x,
                (i32) (e1.position.y + e1.movement.y) + e1.collider.y,
                e1.collider.w, e1.collider.h};

    // collider of e2 currently
    rect_t b = {(i32) e2.position.x + e2.collider.x,
                (i32) e2.position.y + e2.collider.y,
                e2.collider.w, e2.collider.h};

    b32 collided = Collision::AABB(a, b);
    if (collided)
    {
        e1.movement = {0,0,0};
        if ((e1.flags & (u32) EntityFlag::PICKUP_BOX) &&
            (e2.flags & (u32) EntityFlag::IS_ITEM))
        {
            collisionCallback(&e1, &e2,
                              (u32) EntityFlag::PICKUP_BOX |
                              (u32) EntityFlag::IS_ITEM);
        }
    }

    return collided;
}

