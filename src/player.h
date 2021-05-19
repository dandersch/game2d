#pragma once

#include "pch.h"

struct Event;
struct Entity;

class Player
{
public:
    void handleEvent(const Event& e,  Entity& ent);
    void update(f32 dt, Entity& ent);
    void tryMove(glm::vec3 movement, Entity& ent);

private:
    const f32 playerSpeed = 150.f;
};
