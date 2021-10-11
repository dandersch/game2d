#include "physics.h"
#include "base.h"
#include "entity.h"
#include "utils.h"

// array of size 256 (== U8_MAX) for callback functions that we access into
// with a bitmask, e.g. when enemy & spike collide, we do
// callbacks[TYPE_ENEMY & TYPE_SPIKE](e1, e2)
// NOTE (maybe we should  also pass in a struct collision_info)
// NOTE can't be accessed into w/ entityflags that are larger than 8bits
typedef void (*collision_callback_fn)(Entity*, Entity*, s32);
// NOTE not compatible with code hotloading
static collision_callback_fn callbacks[256] = {0}; // 2048 bytes

internal
b32 aabb(const rect_t& recA, const rect_t& recB)
{
    if (utils_rect_intersects(recA, recB)) return true;
    return false;
}

internal // check if two entities share a flag combination
s32 check_flag_combination(Entity* e1, Entity* e2, u32 flag1, u32 flag2)
{
    if ((e1->flags & flag1) && (e2->flags & flag2))
        return 1;
    else if ((e1->flags & flag2) && (e2->flags & flag1))
        return -1;
    else
        return 0;
}

struct flag_combination_t { u8 f1; u8 f2; };
#define PAIRS_SIZE 256
static flag_combination_t all_pairs[PAIRS_SIZE] = {0};
static u32 pair_count = 0;
#define AND_FLAGS(combination) combination.f1 | combination.f2

void physics_init() // fills callbacks array
{
    all_pairs[pair_count] = {ENT_FLAG_PICKUP_BOX, ENT_FLAG_IS_ITEM};
    callbacks[AND_FLAGS(all_pairs[pair_count])] = [](Entity* e1, Entity* e2, s32 dir)
    {
        if (dir == 1) return;
        printf("Pick up item\n");

        e1->owner->item  = e2;
        e2->flags ^= ENT_FLAG_IS_COLLIDER;
    };
    pair_count++;

    all_pairs[pair_count] = {ENT_FLAG_ATTACK_BOX, ENT_FLAG_IS_ITEM};
    callbacks[AND_FLAGS(all_pairs[pair_count])] = [](Entity* e1, Entity* e2, s32 dir)
    {
        printf("Attack item\n");
    };
    pair_count++;

    all_pairs[pair_count] = {ENT_FLAG_PLAYER_CONTROLLED, ENT_FLAG_IS_TILE};
    callbacks[AND_FLAGS(all_pairs[pair_count])] = [](Entity* e1, Entity* e2, s32 dir)
    {
        printf("Wall\n");
    };
    pair_count++;
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
    // TODO only perform collision check if flag combination is present in all_pairs
    // otherwise early out

    // TODO use unpivoted pos or calculate pos back to unpivoted here
    // collider of e1 after moving
    rect_t a = {(i32) (e1.position.x + e1.movement.x) + e1.collider.x,
                (i32) (e1.position.y + e1.movement.y) + e1.collider.y,
                e1.collider.w, e1.collider.h};

    // collider of e2 currently
    // TODO use collider of e2 after moving?
    rect_t b = {(i32) e2.position.x + e2.collider.x,
                (i32) e2.position.y + e2.collider.y,
                e2.collider.w, e2.collider.h};

    b32 collided = aabb(a, b);
    if (collided)
    {
        e1.movement = {0,0,0};
        u8 flag_combination = 0;
        s32 direction = 0; // 1 if e1 acts on e2, -1 if e2 acts on e1, 0 if neither

        for (int i = 0; i < pair_count; i++)
        {
            auto& pair = all_pairs[i];
            if ((direction = check_flag_combination(&e1, &e2, pair.f1, pair.f2)))
                flag_combination = AND_FLAGS(pair);
        }

        if (callbacks[flag_combination] != nullptr)
        {
            callbacks[flag_combination](&e1,&e2, direction);
        }
    }


    // TODO resolve collision here, i.e. push entities out of the intersection box

    return collided;
}
