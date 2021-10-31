#include "camera.h"

// TODO make camera zoom in where the cursor currently is
// TODO zoom in only in steps that are pixel-perfect

v3f camera_screen_to_world(Camera& cam, const v3f& cam_pos)
{
    cam.scale   = (f32) cam.rect.w / SCREEN_WIDTH;
    cam.scale   = 1;
    v3f cam_p   = cam_pos; // NOTE copy because of const (we have no const versions of the overloaded operators)
    v3f scaled_cam_pos = cam_p / cam.scale;
    v3f cam_size       = {{(f32) cam.rect.x, (f32) cam.rect.y, 0}};

    return scaled_cam_pos + cam_size;
}

v3f camera_world_to_screen(Camera& cam, const v3f& world_pos)
{
    cam.scale    = (f32) cam.rect.w / SCREEN_WIDTH;
    v3f cam_size = {(f32) cam.rect.x, (f32) cam.rect.y, 0};
    v3f world    = world_pos; // NOTE copy because of const (we have no const versions of the overloaded operators)

    return (world - cam_size) * cam.scale;
}
