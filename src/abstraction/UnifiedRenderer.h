#pragma once

#include <filesystem>

#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"
#include "Camera.h"
#include "Cubemap.h"
#include "../Utils/AABB.h"
#include "../world/Light/Light.h"

/**
* The Renderer is the primary interface with OpenGL. A final user should not have
* to call any gl function and instead shall use Renderer functions.
* 
* In this namespace are functions that load data (shaders & meshes), initialization
* and finalization methods, debug drawing methods (bounding boxes, cameras...) and
* normal rendering methods (meshes...).
* 
* The Renderer contains a "standard mesh shader" that is set globally and can be
* replaced using #rebuildStandardMeshShader, for example:
*   Renderer::Shader &meshShader = Renderer::rebuildStandardMeshShader(Renderer::ShaderFactory()
*     .prefix("res/shaders/")
*     .addFileVertex("standard.vs")
*     .prefix("mesh_parts/")
*     .addFileFragment("base.fs")
*     .addFileFragment("color_terrain.fs")
*     .addFileFragment("lights_none.fs")
*     .addFileFragment("final_fog.fs")
*     .addFileFragment("shadows_casted.fs")
*     .addFileFragment("normal_none.fs"));
* The "base.fs" shader uses one of each "color_x"|"lights_x"|"final_x"|"shadows_x"|"normal_x"
* parts.
*/
namespace Renderer {
    
namespace fs = std::filesystem;

static struct DebugData {
  int vertexCount; // Does not handle instanced rendering
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

Shader &rebuildStandardMeshShader(const ShaderFactory &builder);
Shader &getStandardMeshShader();
/* Disables color drawing and switches the standard mesh shader to an empty one */
void beginColorPass();
/* Enables color drawing and restores the standard mesh shader */
void beginDepthPass();

void renderMesh(const Camera &camera, const glm::vec3 &position, const glm::vec3 &size, const Mesh& mesh);
void renderNormalsMesh(const Camera &camera, const glm::vec3 &position, const glm::vec3 &size, const NormalsMesh &normalsMesh, const glm::vec4 &color={ 1,0,0,1 });
void renderCubemap(const Camera &camera, const Cubemap &cubemap);
void renderDebugLine(const Camera &camera, const glm::vec3 &from, const glm::vec3 &to, const glm::vec4 &color={1.f, 1.f, 1.f, 1.f});
void renderDebugCube(const Camera &camera, const glm::vec3 &position, const glm::vec3 &size={1.f, 1.f, 1.f}, const glm::vec4 &color={1.f, 1.f, 1.f, 1.f});
void renderDebugAxis(const Camera &camera);
void renderAABBDebugOutline(const Camera &camera, const AABB &aabb, const glm::vec4 &color = { 1.f, 1.f, 0.f, 1.f });
void renderDebugCameraOutline(const Camera &viewCamera, const Camera &outlinedCamera);
void renderDebugGUIQuadWithTexture(const Texture& texture, glm::vec2 positionOnScreen, glm::vec2 size);

void setUniformPointLights(const std::vector<Light>& pointLights);

/**
* A blit pass is a rendering operation that takes an input texture
* (more often than not the texture contains everything that has been
* rendered untill that point) and renders a quad that takes the full
* screen. A single shader is used.
* 
* Blit passes are largelly used for VFX, for example color correction
* can be done using a simple fragment shader in a blit pass.
*/
class BlitPass { // TODO move BlitPass to its own class
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
  void setShader(const fs::path& fs) { m_shader = loadShaderFromFiles("res/shaders/blit.vs", fs); }

  void doBlit(const Texture &renderTexture, bool bindRenderTexture=true); // TODO these parameters are confusing
                                                                          // if the caller needs a texture to be bound it can bind it itself
                                                                          // these two parameters should be removed for clarity
};

}
