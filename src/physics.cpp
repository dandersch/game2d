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

internal_fn
b32 aabb(const rect_t& recA, const rect_t& recB)
{
    if (utils_rect_intersects(recA, recB)) return true;
    return false;
}

internal_fn // check if two entities share a flag combination
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
static flag_combination_t collision_pairs[PAIRS_SIZE] = {0};
static u32 pair_count = 0;
#define AND_FLAGS(combination) combination.f1 | combination.f2

void physics_init() // fills callbacks array
{
    collision_pairs[pair_count++] = {ENT_FLAG_PICKUP_BOX, ENT_FLAG_IS_ITEM};
    collision_pairs[pair_count++] = {ENT_FLAG_ATTACK_BOX, ENT_FLAG_IS_ITEM};
    collision_pairs[pair_count++] = {ENT_FLAG_PLAYER_CONTROLLED, ENT_FLAG_IS_TILE};
}

b32 physics_check_collision_with_tile(Entity& e1, Tile& t1)
{
    // TODO use unpivoted pos or calculate pos back to unpivoted here
    // collider of e1 after moving
    rect_t a = {(i32) (e1.position.x + e1.movement.x) + e1.collider.left,
                (i32) (e1.position.y + e1.movement.y) + e1.collider.top,
                e1.collider.w, e1.collider.h};

    // collider of tile
    rect_t b = {(i32) t1.position.x + t1.collider.left,
                (i32) t1.position.y + t1.collider.top,
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
    // TODO only perform collision check if flag combination is used later on otherwise early out

    // TODO use unpivoted pos or calculate pos back to unpivoted here
    // collider of e1 after moving
    rect_t a = {(i32) (e1.position.x + e1.movement.x) + e1.collider.left,
                (i32) (e1.position.y + e1.movement.y) + e1.collider.top,
                e1.collider.w, e1.collider.h};

    // collider of e2 currently
    // TODO use collider of e2 after moving?
    rect_t b = {(i32) e2.position.x + e2.collider.left,
                (i32) e2.position.y + e2.collider.top,
                e2.collider.w, e2.collider.h};

    b32 collided = aabb(a, b);
    if (!collided) return false;

    e1.movement = {0,0,0};
    u8 flag_combination = 0;
    s32 direction = 0; // 1 if e1 acts on e2, -1 if e2 acts on e1, 0 if neither

    for (int i = 0; i < pair_count; i++)
    {
        auto& pair = collision_pairs[i];
        if ((direction = check_flag_combination(&e1, &e2, pair.f1, pair.f2)))
            flag_combination = AND_FLAGS(pair);
    }

    // TODO indicate direction by jumping to the negative ?
    switch (flag_combination)
    {
        case (ENT_FLAG_PICKUP_BOX | ENT_FLAG_IS_ITEM):
        {
            //if (dir == 1) return;
            printf("Pick up item\n");
            //e1->owner->item  = e2;
            //e2->flags ^= ENT_FLAG_IS_COLLIDER;
        } break;

        case (ENT_FLAG_ATTACK_BOX | ENT_FLAG_IS_ITEM):
        {
            printf("Attack item\n");
        } break;

        case (ENT_FLAG_PLAYER_CONTROLLED | ENT_FLAG_IS_TILE):
        {
            printf("Wall\n");
        } break;
    }

    // if (callbacks[flag_combination] != nullptr) callbacks[flag_combination](&e1,&e2, direction);

    // TODO resolve collision here, i.e. push entities out of the intersection box
    return collided;
}
