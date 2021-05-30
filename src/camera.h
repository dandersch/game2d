#pragma once

#include "pch.h"

// TODO center camera position around center of rect
extern f32 scaleFactor;

class Camera
{
public:
    glm::vec3 screenToWorld(const glm::vec3& pos) const;
    glm::vec3 worldToScreen(const glm::vec3& pos) const;
    SDL_Rect cameraRect = {200, 320, SCREEN_WIDTH, SCREEN_HEIGHT};
};
