#pragma once

#include <filesystem>

#include "Shader.h"
#include "Mesh.h"
#include "Camera.h"
#include "Texture.h"



namespace Renderer {

    static struct debugData {

        int vertexCount;
        int meshCount;
    } s_debugData ;

    
namespace fs = std::filesystem;

Shader loadShaderFromFiles(const fs::path &vertexPath, const fs::path &fragmentPath);
Mesh createCubeMesh(unsigned int texId = 0);
Mesh createPlaneMesh();
Mesh loadMeshFromFile(const fs::path &objPath);

void clear();

void init();
void shutdown();
void clearDebugData();
void showDebugData();

Shader &getStandardMeshShader();

void renderMesh(glm::vec3 position, glm::vec3 size, const Mesh &mesh, const glm::mat4 &VP);
void renderDebugLine(const glm::mat4 &VP, glm::vec3 from, glm::vec3 to, const glm::vec4 &color={1.f, 1.f, 1.f, 1.f});
void renderDebugPlane(const glm::mat4 &VP, const glm::vec3& normal, float point, const glm::vec4 &color={1.f, 1.f, 1.f, 1.f});
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


static void renderDebugPerspectiveCameraOutline(const Camera& viewCamera, const Camera& outlinedCamera)
{
    const PerspectiveProjection& proj = outlinedCamera.getProjection<PerspectiveProjection>();
    glm::vec3 pos = outlinedCamera.getPosition();
    glm::vec3 I = outlinedCamera.getRight();
    glm::vec3 J = outlinedCamera.getUp();
    glm::vec3 F = outlinedCamera.getForward();
    float dh = sin(proj.fovy * .5f);
    float dw = dh * proj.aspect;
    glm::vec3 U1 = F + dh * J + dw * I;
    glm::vec3 U2 = F - dh * J + dw * I;
    glm::vec3 U3 = F - dh * J - dw * I;
    glm::vec3 U4 = F + dh * J - dw * I;
    float zNear = proj.zNear;
    float zFar = proj.zFar;

    renderDebugLine(viewCamera.getViewProjectionMatrix(), pos, pos + F * zFar, { 1.f, 0.f, .0f, 1.f }); // dir
    renderDebugLine(viewCamera.getViewProjectionMatrix(), pos, pos + Camera::UP, { 1.f, 1.f, .3f, 1.f }); // world up
    renderDebugLine(viewCamera.getViewProjectionMatrix(), pos, pos + I, { .5f, 1.f, .5f, 1.f }); // right
    renderDebugLine(viewCamera.getViewProjectionMatrix(), pos, pos + J, { 1.f, .5f, .5f, 1.f }); // up
    renderDebugLine(viewCamera.getViewProjectionMatrix(), pos, pos + U1 * zFar, { .5f, .5f, .5f, 1.f });
    renderDebugLine(viewCamera.getViewProjectionMatrix(), pos, pos + U2 * zFar, { .5f, .5f, .5f, 1.f });
    renderDebugLine(viewCamera.getViewProjectionMatrix(), pos, pos + U3 * zFar, { .5f, .5f, .5f, 1.f });
    renderDebugLine(viewCamera.getViewProjectionMatrix(), pos, pos + U4 * zFar, { .5f, .5f, .5f, 1.f });
    renderDebugLine(viewCamera.getViewProjectionMatrix(), pos + U1 * zFar, pos + U2 * zFar, { .5f, .5f, .5f, 1.f });
    renderDebugLine(viewCamera.getViewProjectionMatrix(), pos + U2 * zFar, pos + U3 * zFar, { .5f, .5f, .5f, 1.f });
    renderDebugLine(viewCamera.getViewProjectionMatrix(), pos + U3 * zFar, pos + U4 * zFar, { .5f, .5f, .5f, 1.f });
    renderDebugLine(viewCamera.getViewProjectionMatrix(), pos + U4 * zFar, pos + U1 * zFar, { .5f, .5f, .5f, 1.f });
    for (float z = zNear; z < zFar; z += 5) {
        renderDebugLine(viewCamera.getViewProjectionMatrix(), pos + U1 * z, pos + U2 * z, { .5f, .5f, .5f, 1.f });
        renderDebugLine(viewCamera.getViewProjectionMatrix(), pos + U2 * z, pos + U3 * z, { .5f, .5f, .5f, 1.f });
        renderDebugLine(viewCamera.getViewProjectionMatrix(), pos + U3 * z, pos + U4 * z, { .5f, .5f, .5f, 1.f });
        renderDebugLine(viewCamera.getViewProjectionMatrix(), pos + U4 * z, pos + U1 * z, { .5f, .5f, .5f, 1.f });
    }
    renderDebugCube(viewCamera.getViewProjectionMatrix(), pos, { .1f, .1f, .1f });
}

static void renderDebugFrustumOutline(const Camera& viewCamera, const Frustum& frustum)
{
    const PerspectiveProjection& proj = viewCamera.getProjection<PerspectiveProjection>();

    glm::vec3 pos = viewCamera.getPosition();
    glm::vec3 I = frustum.rightFace.normal;
    glm::vec3 J = frustum.topFace.normal;
    glm::vec3 F = frustum.nearFace.normal;
    float dh = sin(proj.fovy * .5f);
    float dw = dh * proj.aspect;
    glm::vec3 U1 = F + dh * J + dw * I;
    glm::vec3 U2 = F - dh * J + dw * I;
    glm::vec3 U3 = F - dh * J - dw * I;
    glm::vec3 U4 = F + dh * J - dw * I;
    float zNear = proj.zNear;
    float zFar = proj.zFar;

    renderDebugLine(viewCamera.getViewProjectionMatrix(), pos, pos + F * zFar, { 1.f, 0.f, .0f, 1.f }); // dir
    renderDebugLine(viewCamera.getViewProjectionMatrix(), pos, pos + Camera::UP, { 1.f, 1.f, .3f, 1.f }); // world up
    renderDebugLine(viewCamera.getViewProjectionMatrix(), pos, pos + I, { .5f, 1.f, .5f, 1.f }); // right
    renderDebugLine(viewCamera.getViewProjectionMatrix(), pos, pos + J, { 1.f, .5f, .5f, 1.f }); // up
    renderDebugLine(viewCamera.getViewProjectionMatrix(), pos, pos + U1 * zFar, { .5f, .5f, .5f, 1.f });
    renderDebugLine(viewCamera.getViewProjectionMatrix(), pos, pos + U2 * zFar, { .5f, .5f, .5f, 1.f });
    renderDebugLine(viewCamera.getViewProjectionMatrix(), pos, pos + U3 * zFar, { .5f, .5f, .5f, 1.f });
    renderDebugLine(viewCamera.getViewProjectionMatrix(), pos, pos + U4 * zFar, { .5f, .5f, .5f, 1.f });
    renderDebugLine(viewCamera.getViewProjectionMatrix(), pos + U1 * zFar, pos + U2 * zFar, { .5f, .5f, .5f, 1.f });
    renderDebugLine(viewCamera.getViewProjectionMatrix(), pos + U2 * zFar, pos + U3 * zFar, { .5f, .5f, .5f, 1.f });
    renderDebugLine(viewCamera.getViewProjectionMatrix(), pos + U3 * zFar, pos + U4 * zFar, { .5f, .5f, .5f, 1.f });
    renderDebugLine(viewCamera.getViewProjectionMatrix(), pos + U4 * zFar, pos + U1 * zFar, { .5f, .5f, .5f, 1.f });
    for (float z = zNear; z < zFar; z += 5) {
        renderDebugLine(viewCamera.getViewProjectionMatrix(), pos + U1 * z, pos + U2 * z, { .5f, .5f, .5f, 1.f });
        renderDebugLine(viewCamera.getViewProjectionMatrix(), pos + U2 * z, pos + U3 * z, { .5f, .5f, .5f, 1.f });
        renderDebugLine(viewCamera.getViewProjectionMatrix(), pos + U3 * z, pos + U4 * z, { .5f, .5f, .5f, 1.f });
        renderDebugLine(viewCamera.getViewProjectionMatrix(), pos + U4 * z, pos + U1 * z, { .5f, .5f, .5f, 1.f });
    }
    renderDebugCube(viewCamera.getViewProjectionMatrix(), pos, { .1f, .1f, .1f });
}

}