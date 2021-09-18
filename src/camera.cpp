#include "camera.h"
#include "pch.h"

// TODO make camera zoom in where the cursor currently is
// TODO zoom in only in steps that are pixel-perfect
// TODO get rid of glm::vec3

v3f camera_screen_to_world(Camera& cam, const v3f& cam_pos)
{
    cam.scale =  (f32) cam.rect.w / SCREEN_WIDTH;

    v3f scaled_cam_pos = {cam_pos.x/cam.scale, cam_pos.y/cam.scale, cam_pos.z/cam.scale};
    v3f cam_size       = {(f32) cam.rect.x, (f32) cam.rect.y, 0};

    // TODO v3f_add(x,y) or similar
    return { scaled_cam_pos.x + cam_size.x,
             scaled_cam_pos.y + cam_size.y,
             scaled_cam_pos.z + cam_size.z };
}

v3f camera_world_to_screen(Camera& cam, const v3f& world_pos)
{
    cam.scale    = (f32) cam.rect.w / SCREEN_WIDTH;
    v3f cam_size = {(f32) cam.rect.x, (f32) cam.rect.y, 0};

    return v3f{cam.scale * (world_pos.x - cam_size.x),
               cam.scale * (world_pos.y - cam_size.y),
               cam.scale * (world_pos.z - cam_size.z) };
}
