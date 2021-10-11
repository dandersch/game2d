#include "physics.h"
#include "entity.h"
#include "utils.h"

// array of size 256 (== U8_MAX) for callback functions that we access into
// with a bitmask, e.g. when enemy & spike collide, we do
// callbacks[TYPE_ENEMY & TYPE_SPIKE](e1, e2)
// NOTE (maybe we should  also pass in a struct collision_info)
// NOTE can't be accessed into w/ entityflags that are larger than 8bits
typedef void (*collision_callback_fn)(Entity*, Entity*);
// NOTE not compatible with code hotloading
static collision_callback_fn callbacks[256] = {0}; // 2048 bytes

internal
b32 aabb(const rect_t& recA, const rect_t& recB)
{
    if (rect_intersects(recA, recB)) return true;
    return false;
}

internal // check if two entities share a flag combination
b32 check_flag_combination(Entity* e1, Entity* e2, u32 flag1, u32 flag2)
{
    if ((e1->flags & flag1) && (e2->flags & flag2))
        return true;
    else if ((e1->flags & flag2) && (e2->flags & flag1))
        return true;
    else
        return false;
}

void physics_init() // fills callbacks array
{
    u8 idx = (u8) ((u32) EntityFlag::PICKUP_BOX | (u32) EntityFlag::IS_ITEM);
    callbacks[idx] = [](Entity* e1, Entity* e2)
    {
        printf("Pick up item\n");
        e1->owner->item  = e2;
        e2->flags ^= (u32) EntityFlag::IS_COLLIDER;
    };

    idx = (u8) ((u32) EntityFlag::ATTACK_BOX | (u32) EntityFlag::IS_ITEM);
    callbacks[idx] = [](Entity* e1, Entity* e2) { printf("Attack item\n"); };
}

b32 physics_check_collision_with_tile(Entity& e1, Tile& t1)
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


    b32 collided = aabb(a, b);
    if (collided)
    {
        e1.movement = {0,0,0};

        // check flags here if needed
    }

    return collided;
}

b32 physics_check_collision(Entity& e1, Entity& e2)
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

    b32 collided = aabb(a, b);
    if (collided)
    {
        e1.movement = {0,0,0};
        u8 flag_combination = 0;

        if ((check_flag_combination(&e1, &e2, (u32) EntityFlag::IS_ITEM, (u32) EntityFlag::PICKUP_BOX)))
            flag_combination = (u8) ((u32) EntityFlag::PICKUP_BOX | (u32) EntityFlag::IS_ITEM);

        if ((check_flag_combination(&e1, &e2, (u32) EntityFlag::ATTACK_BOX, (u32) EntityFlag::IS_ITEM)))
            flag_combination = (u8) ((u32) EntityFlag::ATTACK_BOX | (u32) EntityFlag::IS_ITEM);

        if (callbacks[flag_combination] != nullptr)
        {
            callbacks[flag_combination](&e1,&e2);
        }
    }

    return collided;
}
