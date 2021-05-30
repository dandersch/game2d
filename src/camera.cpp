#include "camera.h"
#include "pch.h"

// TODO make camera zoom in where the cursor currently is
// TODO zoom in only in steps that are pixel-perfect

glm::vec3 camera_screen_to_world(Camera& cam, const glm::vec3& cam_pos)
{
    cam.scale =  (f32) cam.rect.w / SCREEN_WIDTH;
    return (cam_pos/cam.scale) + glm::vec3{cam.rect.x, cam.rect.y, 0};
}

glm::vec3 camera_world_to_screen(Camera& cam, const glm::vec3& world_pos)
{
    cam.scale = (f32) cam.rect.w / SCREEN_WIDTH;
    return cam.scale * (world_pos - glm::vec3{cam.rect.x, cam.rect.y, 0});
}
