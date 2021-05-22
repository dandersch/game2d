#pragma once

#include "pch.h"

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

friend class GameLayer;

private:
    static const f32 playerSpeed;
};
