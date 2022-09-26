#pragma once

#include <glm/glm.hpp>

namespace TempRenderer {

void Init();
void RenderCube(glm::vec3 position, glm::vec3 size, glm::vec3 color, const glm::mat4 &VP);

}