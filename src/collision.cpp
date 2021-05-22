#include "collision.h"
#include "entity.h"

static std::map<u32, std::function<void(Entity* e1, Entity* e2)>> callbacks = {
{ (u32) EntityFlag::IS_ENEMY | (u32) EntityFlag::IS_TILE,
  [](Entity* e1, Entity* e2)
  {
      printf("Yes\n");
      // do something
  }},
{ (u32) EntityFlag::PLAYER_CONTROLLED | (u32) EntityFlag::IS_COLLIDER,
  [](Entity* e1, Entity* e2)
  {
      // do something
  }}
};

// NOTE built-in sdl function seems faster
bool Collision::AABB(const SDL_Rect& recA, const SDL_Rect& recB)
{
    // if (recA.x + recA.w >= recB.x &&
    //     recB.x + recB.w >= recA.x &&
    //     recA.y + recA.h >= recB.y &&
    //     recB.y + recB.h >= recA.y)
    // {
    //     return true;
    // }

    //SDL_Rect intersect;
    //if (SDL_IntersectRect(&recA, &recB, NULL))
    if (SDL_HasIntersection(&recA, &recB))
    {
        // TODO use callbacks like this:
        auto cb = callbacks.find((u32) EntityFlag::PLAYER_CONTROLLED | (u32) EntityFlag::IS_TILE);
        if (cb != callbacks.end())
        {
            cb->second(nullptr,nullptr);
        }

        return true;
    }

    return false;
}

bool Collision::checkCollision(Entity& e1, Entity& e2)
{
    // TODO use unpivoted pos or calculate pos back to unpivoted here
    // collider of e1 after moving
    SDL_Rect a = {(i32) (e1.position.x + e1.movement.x) + e1.collider.x,
                  (i32) (e1.position.y + e1.movement.y) + e1.collider.y,
                  e1.collider.w, e1.collider.h,} ;

    // collider of e2 currently
    SDL_Rect b = {(i32) e2.position.x + e2.collider.x,
                  (i32) e2.position.y + e2.collider.y,
                  e2.collider.w, e2.collider.h,} ;

    bool collided = Collision::AABB(a, b);
    if (collided) e1.movement = {0,0,0};

    return collided;
}
