#include "UnifiedRenderer.h"

#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "Window.h"
#include "Camera.h"
#include "Mesh.h"

namespace Renderer {

static struct KeepAliveResources {
  Shader             standardMeshShader;
  Shader             standardLineShader;
  Shader             debugNormalsShader;

  Shader             debugFlatScreenShader;

  Shader             debugCubeShader;
  Mesh               debugCubeMesh;
  VertexArray        lineVAO;
  IndexBufferObject  lineIBO;
  VertexBufferObject emptyVBO; // used by the line vao
} *s_keepAliveResources = nullptr;


Shader loadShaderFromFiles(const fs::path &vertexPath, const fs::path &fragmentPath)
{
  std::ifstream vertexFile{ vertexPath };
  std::ifstream fragmentFile{ fragmentPath };
  std::stringstream buffer;
  buffer << vertexFile.rdbuf();
  std::string vertexCode = buffer.str();
  buffer.str("");
  buffer << fragmentFile.rdbuf();
  std::string fragmentCode = buffer.str();
  return Shader{ vertexCode, fragmentCode };
}

Mesh createCubeMesh(unsigned int texId)
{
  // TODO add UVs
  float s3 = std::sqrtf(3);
  std::vector<Vertex> vertices{
    // position              uv            normal            // tex id          // color
    { { -.5f, -.5f, -.5f }, { 0.f, 0.f }, { -s3, -s3, -s3 }, (float)texId, {1.0f, 1.0f, 0.0f}, },
    { { +.5f, -.5f, -.5f }, { 1.f, 0.f }, { +s3, -s3, -s3 }, (float)texId, {1.0f, 1.0f, 0.0f}, },
    { { +.5f, +.5f, -.5f }, { 1.f, 1.f }, { +s3, +s3, -s3 }, (float)texId, {1.0f, 1.0f, 0.0f}, },
    { { -.5f, +.5f, -.5f }, { 0.f, 1.f }, { -s3, +s3, -s3 }, (float)texId, {1.0f, 1.0f, 0.0f}, },
    { { -.5f, -.5f, +.5f }, { 0.f, 1.f }, { -s3, -s3, +s3 }, (float)texId, {1.0f, 1.0f, 0.0f}, },
    { { +.5f, -.5f, +.5f }, { 1.f, 1.f }, { +s3, -s3, +s3 }, (float)texId, {1.0f, 1.0f, 0.0f}, },
    { { +.5f, +.5f, +.5f }, { 1.f, 0.f }, { +s3, +s3, +s3 }, (float)texId, {1.0f, 1.0f, 0.0f}, },
    { { -.5f, +.5f, +.5f }, { 0.f, 0.f }, { -s3, +s3, +s3 }, (float)texId, {1.0f, 1.0f, 0.0f} },
  };
  std::vector<unsigned int> indices{
    0, 3, 1, 1, 3, 2,
    1, 2, 5, 5, 2, 6,
    5, 6, 4, 4, 6, 7,
    4, 7, 0, 0, 7, 3,
    3, 7, 2, 2, 7, 6,
    4, 0, 5, 5, 0, 1
  };
  return Mesh(vertices, indices);
}

Mesh createPlaneMesh(unsigned int texId)
{
  std::vector<Vertex> vertices{
    // position             uv            normal (up)    // tex id          // color
    { { -.5f, 0.f, -.5f }, { 1.f, 1.f }, { 0, 1.f, 0 }, (float)texId, {1.0f, 1.0f, 0.0f}, },
    { { -.5f, 0.f, +.5f }, { 1.f, 0.f }, { 0, 1.f, 0 }, (float)texId, {1.0f, 1.0f, 0.0f}, },
    { { +.5f, 0.f, +.5f }, { 0.f, 0.f }, { 0, 1.f, 0 }, (float)texId, {1.0f, 1.0f, 0.0f}, },
    { { +.5f, 0.f, -.5f }, { 0.f, 1.f }, { 0, 1.f, 0 }, (float)texId, {1.0f, 1.0f, 0.0f}, },
  };
  std::vector<unsigned int> indices{
    3, 2, 0, 1,0,2
  };
  return Mesh(vertices, indices);
}

static void skipStreamText(std::istream &stream, const char *text)
{
  char c;
  while (*text) {
    if ((c = stream.get()) != *text) {
      std::cerr << "Expected " << *text << " but got " << c;
      throw std::runtime_error("Unexpected charactor got in stream");
    }
    text++;
  }
}

Mesh loadMeshFromFile(const fs::path& objPath)
{
    std::ifstream modelFile{ objPath };
    constexpr size_t bufSize = 10000;
    char lineBuffer[bufSize];

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;

    uvs.emplace_back();

    std::vector<std::tuple<int, int, int>> cachedVertices;
    std::vector<unsigned int> indices;
    std::vector<Vertex> vertices;

  if (!modelFile.good())
    throw std::exception("Could not open model file");

  int l = 0;
  while (modelFile.good()) {
    l++;
    modelFile.getline(lineBuffer, bufSize);
    if (lineBuffer[0] == '#')
      continue;
    int space = 0;
    while (lineBuffer[space] != '\0' && lineBuffer[space] != ' ')
      space++;
    if (lineBuffer[space] == '\0')
      continue;
    std::stringstream ss;
    ss.str(lineBuffer + space + 1);

    float f1, f2, f3;
    int i1, i2, i3;
    if (strstr(lineBuffer, "v ") == lineBuffer) { // vertex
      ss >> f1 >> f2 >> f3;
      positions.push_back({ f1, f2, f3 });
    } else if (strstr(lineBuffer, "vt ") == lineBuffer) { // texture coordinate
      ss >> f1 >> f2;
      uvs.push_back({ f1, f2 });
    } else if (strstr(lineBuffer, "vn ") == lineBuffer) { // normal
      ss >> f1 >> f2 >> f3;
      normals.push_back({ f1, f2, f3 });
    } else if (strstr(lineBuffer, "f ") == lineBuffer) { // face
      for (size_t i = 0; i < 3; i++) {
        ss >> i1; skipStreamText(ss, "/");
        ss >> i2; skipStreamText(ss, "/"); // TODO fix the case where no uvs are specified
        ss >> i3;
        std::tuple<int, int, int> cacheKey{ i1, i2, i3 };
        auto inCacheIndex = std::find(cachedVertices.begin(), cachedVertices.end(), cacheKey);
        if (inCacheIndex == cachedVertices.end()) {
          vertices.emplace_back(positions[i1 - 1ll], uvs[i2 - 1ll], normals[i3 - 1ll]);
          indices.push_back((unsigned int)cachedVertices.size());
          cachedVertices.push_back(cacheKey);
        } else {
          indices.push_back((unsigned int)(inCacheIndex - cachedVertices.begin()));
        }
      }
    } else { // unrecognized line
      continue;
    }
  }

  return Mesh(vertices, indices);
}

void clear()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void init()
{
  VertexBufferLayout emptyLayout;
  s_keepAliveResources = new KeepAliveResources;

  s_keepAliveResources->standardMeshShader = loadShaderFromFiles("res/shaders/standard.vs", "res/shaders/standard_mesh.fs");
  s_keepAliveResources->standardLineShader = loadShaderFromFiles("res/shaders/standard_line.vs", "res/shaders/standard_color.fs");
  
  s_keepAliveResources->lineIBO = IndexBufferObject({ 0, 1 });
  s_keepAliveResources->lineVAO.addBuffer(s_keepAliveResources->emptyVBO, emptyLayout, s_keepAliveResources->lineIBO);

  s_keepAliveResources->debugCubeMesh = createCubeMesh();
  s_keepAliveResources->debugCubeShader = loadShaderFromFiles("res/shaders/standard.vs", "res/shaders/standard_color.fs");
  s_keepAliveResources->debugNormalsShader = loadShaderFromFiles("res/shaders/standard.vs", "res/shaders/standard_color.fs");
  s_keepAliveResources->debugFlatScreenShader = loadShaderFromFiles("res/shaders/debugFlatScreen.vs", "res/shaders/debugFlatScreen.fs");

  VertexArray::unbind();
}

void shutdown()
{
  delete s_keepAliveResources;
}

Shader &getStandardMeshShader()
{
  if (s_keepAliveResources == nullptr)
    throw std::exception("Cannot fetch resources while the renderer is uninitialized");
  return s_keepAliveResources->standardMeshShader;
}

void renderMesh(glm::vec3 position, glm::vec3 size, const Mesh &mesh, const Camera &camera)
{
  s_debugData.meshCount++;
  s_debugData.vertexCount += mesh.getVertexCount();

  glm::mat4 M(1.f);
  M = glm::translate(M, position);
  M = glm::scale(M, size);
  s_keepAliveResources->standardMeshShader.bind();
  s_keepAliveResources->standardMeshShader.setUniform3f("u_cameraPos", camera.getPosition());
  s_keepAliveResources->standardMeshShader.setUniformMat4f("u_M", M);
  s_keepAliveResources->standardMeshShader.setUniformMat4f("u_VP", camera.getViewProjectionMatrix());
  mesh.draw();
}

void renderNormalsMesh(glm::vec3 position, glm::vec3 size, const NormalsMesh &normalsMesh, const Camera &camera, const glm::vec4 &color)
{
  s_debugData.meshCount++;
  s_debugData.debugLines += normalsMesh.getVertexCount()/2;

  glm::mat4 M(1.f);
  M = glm::translate(M, position);
  M = glm::scale(M, size);
  s_keepAliveResources->debugNormalsShader.bind();
  s_keepAliveResources->debugNormalsShader.setUniform4f("u_color", color);
  s_keepAliveResources->debugNormalsShader.setUniformMat4f("u_M", M);
  s_keepAliveResources->debugNormalsShader.setUniformMat4f("u_VP", camera.getViewProjectionMatrix());
  normalsMesh.draw();
}

void renderDebugLine(const glm::mat4 &VP, glm::vec3 from, glm::vec3 to, const glm::vec4 &color)
{
  s_debugData.debugLines++;
  s_debugData.vertexCount += 2;
  s_keepAliveResources->lineVAO.bind();
  s_keepAliveResources->standardLineShader.bind();
  s_keepAliveResources->standardLineShader.setUniform3f("u_from", from);
  s_keepAliveResources->standardLineShader.setUniform3f("u_to", to);
  s_keepAliveResources->standardLineShader.setUniform4f("u_color", color);
  s_keepAliveResources->standardLineShader.setUniformMat4f("u_VP", VP);
  glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, nullptr);
}

void renderDebugCube(const glm::mat4 &VP, glm::vec3 position, glm::vec3 size, const glm::vec4 &color)
{
  glm::mat4 M(1.f);
  M = glm::translate(M, position);
  M = glm::scale(M, size);
  s_keepAliveResources->debugCubeShader.bind();
  s_keepAliveResources->debugCubeShader.setUniform4f("u_color", color);
  s_keepAliveResources->debugCubeShader.setUniformMat4f("u_M", M);
  s_keepAliveResources->debugCubeShader.setUniformMat4f("u_VP", VP);
  s_keepAliveResources->debugCubeMesh.draw();
}

void renderDebugAxis(const glm::mat4 &VP)
{
  renderDebugLine(VP, { 0, 0, 0 }, { 10, 0, 0 }, { 1.f, 0.f, 0.f, 1.f }); // x red
  renderDebugLine(VP, { 0, 0, 0 }, { 0, 10, 0 }, { 0.f, 1.f, 0.f, 1.f }); // y green
  renderDebugLine(VP, { 0, 0, 0 }, { 0, 0, 10 }, { 0.f, 0.f, 1.f, 1.f }); // z blue
}


void renderAABBDebugOutline(const Camera &camera, const AABB &aabb, const glm::vec4 &color)
{
  glm::vec3 o = aabb.getOrigin();
  glm::vec3 x = { aabb.getSize().x, 0, 0 };
  glm::vec3 y = { 0, aabb.getSize().y, 0 };
  glm::vec3 z = { 0, 0, aabb.getSize().z };
  renderDebugLine(camera.getViewProjectionMatrix(), o, o + x, color);
  renderDebugLine(camera.getViewProjectionMatrix(), o + y, o + x + y, color);
  renderDebugLine(camera.getViewProjectionMatrix(), o + z, o + x + z, color);
  renderDebugLine(camera.getViewProjectionMatrix(), o + y + z, o + x + y + z, color);
  renderDebugLine(camera.getViewProjectionMatrix(), o, o + y, color);
  renderDebugLine(camera.getViewProjectionMatrix(), o + x, o + y + x, color);
  renderDebugLine(camera.getViewProjectionMatrix(), o + z, o + y + z, color);
  renderDebugLine(camera.getViewProjectionMatrix(), o + x + z, o + y + x + z, color);
  renderDebugLine(camera.getViewProjectionMatrix(), o, o + z, color);
  renderDebugLine(camera.getViewProjectionMatrix(), o + x, o + z + x, color);
  renderDebugLine(camera.getViewProjectionMatrix(), o + y, o + z + y, color);
  renderDebugLine(camera.getViewProjectionMatrix(), o + x + y, o + z + x + y, color);
}

static void renderDebugOrthographicCameraOutline(const Camera &viewCamera, const Camera &outlinedCamera)
{
  const OrthographicProjection &proj = outlinedCamera.getProjection<OrthographicProjection>();
  const glm::vec3 pos = outlinedCamera.getPosition();
  glm::vec3 I = outlinedCamera.getRight();
  glm::vec3 J = outlinedCamera.getUp();
  glm::vec3 F = outlinedCamera.getForward();
  float zNear = proj.zNear;
  float zFar = proj.zFar;
  glm::vec3 p1 = pos + I * proj.right + J * proj.top;
  glm::vec3 p2 = pos + I * proj.right - J * proj.top;
  glm::vec3 p3 = pos - I * proj.right - J * proj.top;
  glm::vec3 p4 = pos - I * proj.right + J * proj.top;

  renderDebugLine(viewCamera.getViewProjectionMatrix(), pos, pos + F * zFar, { 1.f, 0.f, .0f, 1.f }); // dir
  renderDebugLine(viewCamera.getViewProjectionMatrix(), pos, pos + Camera::UP, { 1.f, 1.f, .3f, 1.f }); // world up
  renderDebugLine(viewCamera.getViewProjectionMatrix(), pos, pos + I, { .5f, 1.f, .5f, 1.f }); // right
  renderDebugLine(viewCamera.getViewProjectionMatrix(), pos, pos + J, { 1.f, .5f, .5f, 1.f }); // up
  renderDebugLine(viewCamera.getViewProjectionMatrix(), p1, p1 + F * zFar, { .5f, .5f, .5f, 1.f });
  renderDebugLine(viewCamera.getViewProjectionMatrix(), p2, p2 + F * zFar, { .5f, .5f, .5f, 1.f });
  renderDebugLine(viewCamera.getViewProjectionMatrix(), p3, p3 + F * zFar, { .5f, .5f, .5f, 1.f });
  renderDebugLine(viewCamera.getViewProjectionMatrix(), p4, p4 + F * zFar, { .5f, .5f, .5f, 1.f });
  for (float z = zNear; z < zFar; z += 5) {
    renderDebugLine(viewCamera.getViewProjectionMatrix(), p1 + z * F, p2 + z * F, { .5f, .5f, .5f, 1.f });
    renderDebugLine(viewCamera.getViewProjectionMatrix(), p2 + z * F, p3 + z * F, { .5f, .5f, .5f, 1.f });
    renderDebugLine(viewCamera.getViewProjectionMatrix(), p3 + z * F, p4 + z * F, { .5f, .5f, .5f, 1.f });
    renderDebugLine(viewCamera.getViewProjectionMatrix(), p4 + z * F, p1 + z * F, { .5f, .5f, .5f, 1.f });
  }
  renderDebugCube(viewCamera.getViewProjectionMatrix(), pos, { .1f, .1f, .1f });
}

static void renderDebugPerspectiveCameraOutline(const Camera &viewCamera, const Camera &outlinedCamera)
{
  const PerspectiveProjection &proj = outlinedCamera.getProjection<PerspectiveProjection>();
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

void renderDebugCameraOutline(const Camera &viewCamera, const Camera &outlinedCamera)
{
  switch (outlinedCamera.getProjectionType()) {
  case CameraProjection::ORTHOGRAPHIC: renderDebugOrthographicCameraOutline(viewCamera, outlinedCamera);       break;
  case CameraProjection::PERSPECTIVE:  renderDebugPerspectiveCameraOutline(viewCamera, outlinedCamera); break;
  default:                             throw std::exception("Unreachable");                             break;
  }
}


void renderDebugGUIQuadWithTexture(const Camera& camera, const Texture& texture, glm::vec2 positionOnScreen, glm::vec2 size)
{

    std::vector<Vertex> vertices{
        // position             uv            normal (up)    // tex id          // color
        { { positionOnScreen.x,         positionOnScreen.y, 0.f }               , { 1.f, 1.f }, { 0, 1.f, 0 }, 0, {1.0f, 1.0f, 0.0f}, },
        { { positionOnScreen.x,         positionOnScreen.y + size.y, 0.f }      , { 1.f, 0.f }, { 0, 1.f, 0 }, 0, {1.0f, 1.0f, 0.0f}, },
        { {positionOnScreen.x + size.x, positionOnScreen.y + size.y, 0.f }      , { 0.f, 0.f }, { 0, 1.f, 0 }, 0, {1.0f, 1.0f, 0.0f}, },
        { {positionOnScreen.x + size.x, positionOnScreen.y, 0.f }               , { 0.f, 1.f }, { 0, 1.f, 0 }, 0, {1.0f, 1.0f, 0.0f}, },
    };

    std::vector<unsigned int> indices{
      3, 2, 0, 1,0,2
    };

    Mesh gui{ vertices, indices };

    s_keepAliveResources->debugFlatScreenShader.bind();
    texture.bind(0);
    s_keepAliveResources->debugFlatScreenShader.setUniform1i("u_texture", 0);
    gui.draw();

}

//=========================================================================================================================//
//=========================================================================================================================//
//=========================================================================================================================//
//=========================================================================================================================//
BlitPass::BlitPass()
  : BlitPass("res/shaders/blit.fs")
{
}

BlitPass::BlitPass(const fs::path &fragmentShaderPath)
{
  m_shader = loadShaderFromFiles("res/shaders/blit.vs", fragmentShaderPath);
  m_keepAliveIBO = IndexBufferObject({ 0, 2, 1, 3, 2, 0 });
  m_vao.addBuffer(m_keepAliveVBO, VertexBufferLayout{}, m_keepAliveIBO);
}

void BlitPass::doBlit(const Texture &renderTexture, bool bindRenderTexture/*=true*/)
{
  glDisable(GL_DEPTH_TEST);
  Renderer::clear();
  if (bindRenderTexture)
      renderTexture.bind();

  m_shader.bind();
  //m_shader.setUniform2f("u_screenSize", (float)Window::getWinWidth(), (float)Window::getWinHeight()); // TODO remove, this uniform is only necessary because the vignette vfx is in the blit shader, which it shouldn't, it should be in a res/shaders/testfb_blit.fs shader
  m_vao.bind();
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
  Texture::unbind();
  VertexArray::unbind();
  glEnable(GL_DEPTH_TEST);
}


void clearDebugData() {
    s_debugData.meshCount = 0;
    s_debugData.vertexCount = 0;
    s_debugData.debugLines = 0;
}

const DebugData& getRendererDebugData() {
    return s_debugData;
}

}