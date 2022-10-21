#include "Sky.h"

#include <glm/glm.hpp>
#include "../vendor/imgui/imgui.h"

#include "../abstraction/Shader.h"
#include "../abstraction/VertexBufferObject.h"
#include "../abstraction/IndexBufferObject.h"
#include "../abstraction/Shader.h"
#include "../abstraction/Cubemap.h"
#include "../World/Player.h"
#include "../Utils/Mathf.h"
#include "../abstraction/UnifiedRenderer.h"
#include "TerrainGeneration/MapUtilities.h"
#include "TerrainGeneration/Noise.h"

namespace World {

Sky::Sky()
  : m_skybox("res/skybox_dbg/skybox_front.bmp", "res/skybox_dbg/skybox_back.bmp",
             "res/skybox_dbg/skybox_left.bmp", "res/skybox_dbg/skybox_right.bmp",
             "res/skybox_dbg/skybox_top.bmp", "res/skybox_dbg/skybox_bottom.bmp")
{
  HeightMap noisemap;
  unsigned int textureSize = 300;
  noisemap.setHeights(textureSize, textureSize, Noise::generateNoiseMap(textureSize, textureSize, 10.f, 4, .5f, 1.5f, 0));
  m_cloudsTexture = MapUtilities::genTextureFromHeightmap(noisemap);
}

void Sky::step(float delta)
{
}

void Sky::render(const Player &player) const
{
  Renderer::CubemapRenderer::drawCubemap(m_skybox, player.getCamera());
  Renderer::SkyRenderer::drawSkyClouds(m_cloudsTexture, player);
}

}


namespace Renderer::SkyRenderer {

static struct KeepAliveResources {
  Mesh planeMesh;
  Shader cloudsShader;
} *keepAliveResources;

static Renderer::Mesh createPlaneMesh()
{
  std::vector<Renderer::Vertex> vertices;
  std::vector<unsigned int> indices;
  //                   position           uv            normal (up)
  vertices.push_back({ {-.5f, 0.f, -.5f}, { 0.f, 0.f }, { 0, 1.f, 0 } });
  vertices.push_back({ {+.5f, 0.f, -.5f}, { 1.f, 0.f }, { 0, 1.f, 0 } });
  vertices.push_back({ {+.5f, 0.f, +.5f}, { 1.f, 1.f }, { 0, 1.f, 0 } });
  vertices.push_back({ {-.5f, 0.f, +.5f}, { 0.f, 1.f }, { 0, 1.f, 0 } });
  indices.push_back(0); indices.push_back(1); indices.push_back(2);
  indices.push_back(2); indices.push_back(3); indices.push_back(0);
  return Renderer::Mesh(vertices, indices);
}

void init()
{
  keepAliveResources = new KeepAliveResources;
  keepAliveResources->planeMesh = Renderer::createPlaneMesh();

  keepAliveResources->cloudsShader = Shader(R"glsl(
#version 330 core

layout(location = 0) in vec3 i_position;
layout(location = 1) in vec2 i_uv;
layout(location = 2) in vec3 i_normal;

out vec2 o_uv;

uniform mat4 u_VP;
uniform mat4 u_M;

void main()
{
  o_uv = i_uv;
  gl_Position = u_VP * u_M * vec4(i_position, +1.0);
}
)glsl", R"glsl(
#version 330 core

out vec4 color;

in vec2 o_uv;

uniform sampler2D u_texture;

float easeOut(float x) {
  return 1-(1-x)*(1-x);
}

void main()
{
  color = vec4(1.);

  float w = texture(u_texture, o_uv).x;
  vec2 uv = o_uv*2.-1.;
  color.a = easeOut(w * (1-dot(uv, uv)));
}
)glsl");
}

void drawSkyClouds(const Texture &cloudsTexture, const Player &player)
{
  glDepthMask(false); // do not write to depth buffer
  glm::mat4 M(1.f);
  // beware! if vertices go too far outside the clip range after the vertex shader
  // transformation, some may flicker, making triangles break and the whole plane
  // falling appart
  M = glm::translate(M, player.getPosition() + glm::vec3{ 0, .3f, 0 });
  M = glm::scale(M, { 5.f, 1.f, 5.f });
  keepAliveResources->cloudsShader.bind();
  keepAliveResources->cloudsShader.setUniformMat4f("u_M", M);
  keepAliveResources->cloudsShader.setUniformMat4f("u_VP", player.getCamera().getViewProjectionMatrix());
  cloudsTexture.bind();
  keepAliveResources->planeMesh.draw();
  //cloudsTexture.bind();
  //Renderer::RenderMesh(player.getPosition() + glm::vec3{ 0, .3f, 0 }, { 5.f, 1.f, 5.f }, keepAliveResources->planeMesh, player.getCamera().getViewProjectionMatrix());
  glDepthMask(true);
}

}