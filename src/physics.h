#pragma once

struct Entity;
struct Tile;

// TODO for all entities collect their desired movement in one update loop,
// then have one loop for collision detection with that movement vec

void physics_init();
b32  physics_check_collision(Entity& e1, Entity& e2);
b32  physics_check_collision_with_tile(Entity& e1, Tile& t1);
