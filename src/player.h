#pragma once

struct Command
{
    enum Type
    {
        MOVE,
        PICKUP,
        ATTACK
    } type = MOVE;

    // TODO use union in case we need more parameters
    v3f movement = {0,0,0};
};

struct Entity;
struct Camera;

void player_update(f32 dt, Entity& ent);
void player_try_move(v3f movement, Entity& ent);
void player_try_pickup(v3f direction, Entity& ent);
void player_try_attack(v3f direction, Entity& ent);
