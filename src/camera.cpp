#include "camera.h"

glm::vec3 Camera::screenToWorld(const glm::vec3& pos) const
{
    return pos + glm::vec3{cameraRect.x, cameraRect.y, 0};
}

glm::vec3 Camera::worldToScreen(const glm::vec3& pos) const
{
    return pos - glm::vec3{cameraRect.x, cameraRect.y, 0};
}
