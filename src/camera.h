#pragma once

// TODO center camera position around center of rect

extern u32 SCREEN_WIDTH;
extern u32 SCREEN_HEIGHT;

struct Camera
{
    // TODO use instead:
    // v2f topleft;
    // v2i size;
    rect_t rect    = {200, 320, (i32) SCREEN_WIDTH, (i32) SCREEN_HEIGHT};
    f32    scale = 1.f;

    // for lerping
    rect_t target = rect; // TODO this needs to set for lerping right now
    rect_t source;
    f32 timer        = 0.0f;
    f32 TIME_TO_LERP = 0.5f;
};

v3f camera_screen_to_world(Camera& cam, const v3f& cam_pos);
v3f camera_world_to_screen(Camera& cam, const v3f& world_pos);

void camera_zoom(Camera& cam, f32 zoom_factor, v2i zoom_pos);
