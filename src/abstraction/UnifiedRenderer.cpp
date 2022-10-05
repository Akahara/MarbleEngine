#include "UnifiedRenderer.h"

#include <fstream>
#include <sstream>
#include <iostream>

static float lol = 0.001;
static float step = 0.002;

namespace Renderer {

static struct KeepAliveResources {
  Shader standardMeshShader;
} *s_keepAliveResources;


Shader LoadShaderFromFiles(const fs::path &vertexPath, const fs::path &fragmentPath)
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

Mesh CreateCubeMesh()
{
  // TODO add UVs
  float s3 = std::sqrtf(3);
  std::vector<Vertex> vertices{
    // position              uv            normal
    { { -.5f, -.5f, -.5f }, { 0.f, 0.f }, { -s3, -s3, -s3 } },
    { { +.5f, -.5f, -.5f }, { 0.f, 0.f }, { +s3, -s3, -s3 } },
    { { +.5f, +.5f, -.5f }, { 0.f, 0.f }, { +s3, +s3, -s3 } },
    { { -.5f, +.5f, -.5f }, { 0.f, 0.f }, { -s3, +s3, -s3 } },
    { { -.5f, -.5f, +.5f }, { 0.f, 0.f }, { -s3, -s3, +s3 } },
    { { +.5f, -.5f, +.5f }, { 0.f, 0.f }, { +s3, -s3, +s3 } },
    { { +.5f, +.5f, +.5f }, { 0.f, 0.f }, { +s3, +s3, +s3 } },
    { { -.5f, +.5f, +.5f }, { 0.f, 0.f }, { -s3, +s3, +s3 } },
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

Mesh CreatePlaneMesh()
{
  std::vector<Renderer::Vertex> vertices{
    // position             uv            normal (up)
    { { +.5f, 0.f, +.5f }, { 0.f, 0.f }, { 0, 1.f, 0 } },
    { { -.5f, 0.f, +.5f }, { 1.f, 0.f }, { 0, 1.f, 0 } },
    { { -.5f, 0.f, -.5f }, { 1.f, 1.f }, { 0, 1.f, 0 } },
    { { +.5f, 0.f, -.5f }, { 0.f, 1.f }, { 0, 1.f, 0 } },
  };
  std::vector<unsigned int> indices{
    0, 1, 2, 2, 3, 0
  };
  return Mesh(vertices, indices);
}

void Init()
{
  s_keepAliveResources = new KeepAliveResources;
  s_keepAliveResources->standardMeshShader = LoadShaderFromFiles("res/shaders/standard.vs", "res/shaders/standard_mesh.fs");
}

void Shutdown()
{
  delete s_keepAliveResources;
}

void RenderMesh(glm::vec3 position, glm::vec3 size, const Mesh &mesh, const glm::mat4 &VP)
{
  glm::mat4 M(1.f);
  M = glm::translate(M, position);
  M = glm::scale(M, size);
  s_keepAliveResources->standardMeshShader.Bind();
  s_keepAliveResources->standardMeshShader.SetUniformMat4f("u_M", M);
  s_keepAliveResources->standardMeshShader.SetUniformMat4f("u_VP", VP);
  if (lol > 1 || lol < 0) step = -step;
  lol += step;
  s_keepAliveResources->standardMeshShader.SetUniform1f("u_Strenght", lol);
  mesh.Draw();
}

}