#pragma once

#include <glm/glm.hpp>

#include "Mesh.h"

namespace TempRenderer {

void Init();
void RenderCube(glm::vec3 position, glm::vec3 size, glm::vec3 color, const glm::mat4 &VP); 
void RenderPlane(const glm::vec3& position, const glm::vec3& size, const glm::vec3& color, float rotation, const glm::mat4& VP, bool drawLines=false);
void RenderMesh(glm::vec3 position, glm::vec3 size, const Renderer::Mesh &mesh, const glm::mat4 &VP);

}