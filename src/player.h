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

class Player
{
public:
    Player() = delete;
    static void handleEvent(const Event& e,  Entity& ent, const Camera& cam);
    static void update(f32 dt, Entity& ent);
    static void tryMove(glm::vec3 movement, Entity& ent);
    static void tryPickUp(glm::vec3 direction, Entity& ent);

friend class GameLayer;

private:
    static const f32 playerSpeed;
};
