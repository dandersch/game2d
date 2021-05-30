#pragma once

#include "pch.h"

struct Command
{
    enum Type
    {
        MOVE,
        PICKUP,
        ATTACK
    } type = MOVE;

    // TODO use union in case we need more parameters
    glm::vec3 movement = {0,0,0};
};

struct Event;
struct Entity;
struct Camera;

void player_handle_event(const Event& e,  Entity& ent, const Camera& cam);
void player_update(f32 dt, Entity& ent);
void player_try_move(glm::vec3 movement, Entity& ent);
void player_try_pickup(glm::vec3 direction, Entity& ent);
void player_try_attack(glm::vec3 direction, Entity& ent);
