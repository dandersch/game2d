#pragma once

#include "pch.h"

// TODO center camera position around center of rect

struct Camera
{
    SDL_Rect rect  = {200, 320, SCREEN_WIDTH, SCREEN_HEIGHT};
    f32      scale = 1.f;
};

glm::vec3 camera_screen_to_world(Camera& cam, const glm::vec3& cam_pos);
glm::vec3 camera_world_to_screen(Camera& cam, const glm::vec3& world_pos);
