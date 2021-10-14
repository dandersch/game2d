#pragma once

// TODO center camera position around center of rect

extern const u32 SCREEN_WIDTH;
extern const u32 SCREEN_HEIGHT;

struct Camera
{
    rect_t rect  = {200, 320, (i32) SCREEN_WIDTH, (i32) SCREEN_HEIGHT};
    f32    scale = 1.f;
};

v3f camera_screen_to_world(Camera& cam, const v3f& cam_pos);
v3f camera_world_to_screen(Camera& cam, const v3f& world_pos);
