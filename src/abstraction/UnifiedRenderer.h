#pragma once

#include <filesystem>

#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"



namespace Renderer {

namespace fs = std::filesystem;

Shader loadShaderFromFiles(const fs::path &vertexPath, const fs::path &fragmentPath);
Mesh createCubeMesh();
Mesh createPlaneMesh();
Mesh loadMeshFromFile(const fs::path &objPath);

void clear();

void init();
void shutdown();

Shader &getStandardMeshShader();

void renderMesh(glm::vec3 position, glm::vec3 size, const Mesh &mesh, const glm::mat4 &VP);
void renderDebugLine(const glm::mat4 &VP, glm::vec3 from, glm::vec3 to, const glm::vec4 &color={1.f, 1.f, 1.f, 1.f});
void renderDebugCube(const glm::mat4 &VP, glm::vec3 position, glm::vec3 size={1.f, 1.f, 1.f}, const glm::vec4 &color={1.f, 1.f, 1.f, 1.f});
void renderDebugAxis(const glm::mat4 &VP);

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

  Shader &getShader() { return m_shader; }

  void doBlit(const Texture &renderTexture);
};

}