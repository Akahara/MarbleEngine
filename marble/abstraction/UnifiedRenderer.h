#pragma once

#include <filesystem>
#include <array>

#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"
#include "Camera.h"
#include "Cubemap.h"
#include "../Utils/AABB.h"
#include "../World/Light/Light.h"

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


enum RenderingState {

    FORWARD,
    DEFERRED,
    FORWARD_PLUS
};


RenderingState getCurrentRenderingState();
void setRenderingState(RenderingState state);

Shader loadShaderFromFiles(const fs::path &vertexPath, const fs::path &fragmentPath);
Mesh createCubeMesh();
Mesh createSphereMesh(int resolution = 10);

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


// Change this for quaternions
enum ROTATION_AXIS {
    X_AXIS = 1,
    Y_AXIS = 2,
    Z_AXIS = 4,
    XY_AXIS = X_AXIS | Y_AXIS,
    XZ_AXIS = X_AXIS | Z_AXIS,
    YZ_AXIS = Y_AXIS | Z_AXIS,
    XYZ_AXIS = X_AXIS |Y_AXIS | Z_AXIS
};

struct Rotation {
    unsigned char axis;
    float theta;

    Rotation(unsigned char a, float t) : axis(a), theta(t) {}
};

/* For rotation, give a initialization of a struct like that = {axis, degrees}, where axis is the enum Renderer::ROTATION_AXIS (0=x, 1=y, 2=z), use bitwise or built enum*/
void renderMesh(const Camera& camera, const glm::vec3& position, const glm::vec3& size, const Mesh& mesh, const Rotation& rotation = { X_AXIS, 0.F });
void renderNormalsMesh(const Camera &camera, const glm::vec3 &position, const glm::vec3 &size, const NormalsMesh &normalsMesh, const glm::vec4 &color={ 1,0,0,1 });
void renderCubemap(const Camera &camera, const Cubemap &cubemap);
void renderDebugLine(const Camera &camera, const glm::vec3 &from, const glm::vec3 &to, const glm::vec4 &color={1.f, 1.f, 1.f, 1.f});
void renderDebugCube(const Camera &camera, const glm::vec3 &position, const glm::vec3 &size={1.f, 1.f, 1.f}, const glm::vec4 &color={1.f, 1.f, 1.f, 1.f});
void renderDebugAxis(const Camera &camera);
void renderAABBDebugOutline(const Camera &camera, const AABB &aabb, const glm::vec4 &color = { 1.f, 1.f, 0.f, 1.f });
void renderDebugCameraOutline(const Camera &viewCamera, const Camera &outlinedCamera);
void renderDebugGUIQuadWithTexture(const Texture& texture, glm::vec2 positionOnScreen, glm::vec2 size);

void setUniformPointLights(const std::vector<Light>& pointLights);

}
