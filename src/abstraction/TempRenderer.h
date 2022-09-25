#pragma once

#include <glm/glm.hpp>

namespace TempRenderer {

void Init();
void RenderCube(glm::vec3 position, glm::vec3 size, glm::vec3 color, const glm::mat4 &VP); 
void RenderPlane(const glm::vec3& position, const glm::vec3& size, const glm::vec3& color, float rotation, const glm::mat4& VP, bool drawLines);
void RenderFlatPlane(const glm::vec3& position, const glm::vec3& size, const glm::vec3& color, float rotation, const glm::mat4& VP, bool drawLines);

}