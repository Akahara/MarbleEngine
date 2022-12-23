#pragma once

#include <filesystem>

#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"
#include "Camera.h"
#include "Cubemap.h"
#include "../Utils/AABB.h"
#include "../world/Light/Light.h"


namespace Renderer {
    
namespace fs = std::filesystem;

static struct DebugData {
  int vertexCount;
  int meshCount;
  int debugLines;
} s_debugData;

Shader loadShaderFromFiles(const fs::path &vertexPath, const fs::path &fragmentPath);
Mesh createCubeMesh();
Mesh createPlaneMesh(bool facingDown=false);
Mesh loadMeshFromFile(const fs::path &objPath);

void clear();

void init();
void shutdown();
void clearDebugData();
const DebugData& getRendererDebugData();

Shader &getStandardMeshShader();
void beginColorPass();
void beginDepthPass();

void renderMesh(const Camera &camera, const glm::vec3 &position, const glm::vec3 &size, const Mesh& mesh);
void renderNormalsMesh(const Camera &camera, const glm::vec3 &position, const glm::vec3 &size, const NormalsMesh &normalsMesh, const glm::vec4 &color={ 1,0,0,1 });
void renderCubemap(const Camera &camera, const Cubemap &cubemap);
void renderDebugLine(const Camera &camera, const glm::vec3 &from, const glm::vec3 &to, const glm::vec4 &color={1.f, 1.f, 1.f, 1.f});
void renderDebugPlane(const Camera &camera, const glm::vec3 &normal, float point, const glm::vec4 &color={1.f, 1.f, 1.f, 1.f});
void renderDebugCube(const Camera &camera, const glm::vec3 &position, const glm::vec3 &size={1.f, 1.f, 1.f}, const glm::vec4 &color={1.f, 1.f, 1.f, 1.f});
void renderDebugAxis(const Camera &camera);
void renderAABBDebugOutline(const Camera &camera, const AABB &aabb, const glm::vec4 &color = { 1.f, 1.f, 0.f, 1.f });
void renderDebugCameraOutline(const Camera &viewCamera, const Camera &outlinedCamera);
void renderDebugGUIQuadWithTexture(const Texture& texture, glm::vec2 positionOnScreen, glm::vec2 size);

void setUniformPointLights(const std::vector<Light>& pointLights);

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
