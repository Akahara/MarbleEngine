#pragma once

#include <filesystem>

#include "Shader.h"
#include "Mesh.h"



namespace Renderer {

namespace fs = std::filesystem;

Shader LoadShaderFromFiles(const fs::path &vertexPath, const fs::path &fragmentPath);
Mesh CreateCubeMesh();
Mesh CreatePlaneMesh();

void Init();
void Shutdown();

void RenderMesh(glm::vec3 position, glm::vec3 size, const Mesh &mesh, const glm::mat4 &VP);

Shader& getShader();

}