#pragma once

#include <filesystem>

#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"
#include "Camera.h"

#include "../world/Light/Light.h"


namespace Renderer {
    
namespace fs = std::filesystem;

static struct DebugData {
  int vertexCount;
  int meshCount;
  int debugLines;
} s_debugData;

Shader loadShaderFromFiles(const fs::path &vertexPath, const fs::path &fragmentPath);
Mesh createCubeMesh(unsigned int texId = 0);
Mesh createPlaneMesh(unsigned int texId = 0);
Mesh loadMeshFromFile(const fs::path &objPath);

void clear();

void init();
void shutdown();
void clearDebugData();
const DebugData& getRendererDebugData();

Shader &getStandardMeshShader();

void setUniformPointLight(Light pointLight);

// TODO pass cameras in debug rendering functions instead of VP matrices
void renderMesh(glm::vec3 position, glm::vec3 size, const Mesh& mesh, const Camera& camera);
void renderNormalsMesh(glm::vec3 position, glm::vec3 size, const NormalsMesh &normalsMesh, const Camera &camera, const glm::vec4 &color={ 1,0,0,1 });
void renderDebugLine(const glm::mat4 &VP, glm::vec3 from, glm::vec3 to, const glm::vec4 &color={1.f, 1.f, 1.f, 1.f});
void renderDebugPlane(const glm::mat4 &VP, const glm::vec3& normal, float point, const glm::vec4 &color={1.f, 1.f, 1.f, 1.f});
void renderDebugCube(const glm::mat4 &VP, glm::vec3 position, glm::vec3 size={1.f, 1.f, 1.f}, const glm::vec4 &color={1.f, 1.f, 1.f, 1.f});
void renderDebugAxis(const glm::mat4 &VP);
void renderAABBDebugOutline(const Camera &camera, const AABB &aabb, const glm::vec4 &color = { 1.f, 1.f, 0.f, 1.f });
void renderDebugCameraOutline(const Camera &viewCamera, const Camera &outlinedCamera);
void renderDebugGUIQuadWithTexture(const Texture& texture, glm::vec2 positionOnScreen, glm::vec2 size);

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
  void setShader(const fs::path& fs) {
          m_shader = loadShaderFromFiles("res/shaders/blit.vs", fs);

  }

  void doBlit(const Texture &renderTexture, bool bindRenderTexture=true);
};

}
