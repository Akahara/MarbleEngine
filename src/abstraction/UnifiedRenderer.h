#pragma once

#include <filesystem>

#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"

namespace Renderer {

namespace fs = std::filesystem;

Shader LoadShaderFromFiles(const fs::path &vertexPath, const fs::path &fragmentPath);
Mesh CreateCubeMesh();
Mesh CreatePlaneMesh();
Mesh LoadMeshFromFile(const fs::path &objPath);

void Clear();

void Init();
void Shutdown();

void RenderMesh(glm::vec3 position, glm::vec3 size, const Mesh &mesh, const glm::mat4 &VP);
void RenderDebugLine(const glm::mat4 &VP, glm::vec3 from, glm::vec3 to, const glm::vec4 &color={1.f, 1.f, 1.f, 1.f});
void RenderDebugCube(const glm::mat4 &VP, glm::vec3 position, glm::vec3 size={1.f, 1.f, 1.f}, const glm::vec4 &color={1.f, 1.f, 1.f, 1.f});
void RenderDebugAxis(const glm::mat4 &VP);

class BlitPass {
private:
  IndexBufferObject  m_keepAliveIBO;
  VertexBufferObject m_keepAliveVBO;

  Shader             m_shader;
  VertexArray        m_vao;
public:
  BlitPass();
  BlitPass(const fs::path &fragmentShaderPath);
  BlitPass(const BlitPass &) = delete;
  BlitPass &operator=(const BlitPass &) = delete;

  Shader &GetShader() { return m_shader; }

  void DoBlit(const Texture &renderTexture);
};

}