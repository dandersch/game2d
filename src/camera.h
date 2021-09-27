#pragma once

#include "globals.h"

// TODO center camera position around center of rect

struct Camera
{
    rect_t rect  = {200, 320, SCREEN_WIDTH, SCREEN_HEIGHT};
    f32    scale = 1.f;
};

v3f camera_screen_to_world(Camera& cam, const v3f& cam_pos);
v3f camera_world_to_screen(Camera& cam, const v3f& world_pos);
