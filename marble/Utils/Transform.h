#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

struct Transform {
    glm::vec3 position{0,0,0};
    glm::vec3 scale{1,1,1};
    glm::quat rotation{};
};