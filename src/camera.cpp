#include "camera.h"
#include "pch.h"

//  TODO
f32 scaleFactor = 1.f;

// TODO make camera zoom in where the cursor currently is
// TODO zoom in only in steps that are pixel-perfect

glm::vec3 Camera::screenToWorld(const glm::vec3& pos) const
{
    scaleFactor =  (f32) cameraRect.w / SCREEN_WIDTH;
    return (pos/scaleFactor) + glm::vec3{cameraRect.x, cameraRect.y, 0};
}

glm::vec3 Camera::worldToScreen(const glm::vec3& pos) const
{
    scaleFactor = (f32) cameraRect.w / SCREEN_WIDTH;
    return scaleFactor * (pos - glm::vec3{cameraRect.x, cameraRect.y, 0});
}
