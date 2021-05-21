#include "collision.h"
#include "entity.h"

bool Collision::AABB(const SDL_Rect& recA, const SDL_Rect& recB)
{
    if (recA.x + recA.w >= recB.x &&
        recB.x + recB.w >= recA.x &&
        recA.y + recA.h >= recB.y &&
        recB.y + recB.h >= recA.y)
    {
        return true;
    }

    return false;
}

bool Collision::checkCollision(Entity& e1, Entity& e2)
{
    // collider of e1 after moving
    SDL_Rect a = {(i32) (e1.position.x + e1.movement.x) + e1.collider.x,
                  (i32) (e1.position.y + e1.movement.y) + e1.collider.y,
                  e1.collider.w, e1.collider.h,} ;
    // collider of e2 currently
    SDL_Rect b = {(i32) e2.position.x + e2.collider.x,
                  (i32) e2.position.y + e2.collider.y,
                  e2.collider.w, e2.collider.h,} ;

    bool collided = Collision::AABB(a, b);
    if (collided)
        e1.movement = {0,0,0};
    return collided;
}
