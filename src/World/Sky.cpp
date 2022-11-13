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
}

void Sky::render(const Renderer::Camera &camera, float time) const
{
  Renderer::renderCubemap(camera, m_skybox);
  Renderer::SkyRenderer::drawSkyClouds(camera, time);
}

}


// TODO move the sky renderer?
namespace Renderer::SkyRenderer {

static struct KeepAliveResources {
  Mesh planeMesh;
  Shader cloudsShader;
} *keepAliveResources;

void init()
{
  keepAliveResources = new KeepAliveResources;
  keepAliveResources->planeMesh = Renderer::createPlaneMesh(true);

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

uniform float u_time;
uniform vec2 u_worldOffset;

float easeOut(float x) {
  return 1-(1-x)*(1-x);
}

// https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
float hash(vec2 p) {
  return fract(1e4 * sin(dot(p, vec2(17., .1))) * (.1 + abs(sin(dot(p, vec2(13., 1.))))));
}

float noise(vec2 x) {
	vec2 i = floor(x);
	vec2 f = fract(x);

	float a = hash(i);
	float b = hash(i + vec2(1.0, 0.0));
	float c = hash(i + vec2(0.0, 1.0));
	float d = hash(i + vec2(1.0, 1.0));

	vec2 u = f * f * (3.0 - 2.0 * f);
	return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

void main()
{
  vec2 uv = o_uv*2.-1.;
  float t = u_time * .1;
  vec2 o = uv*3 - t * .01 - u_worldOffset * .0003;
  float height = 0;
  float amplitude = .5;
  float lacunarity = 10;
  for(int i = 0; i < 5; i++) {
	height += noise(o * lacunarity + vec2(cos(i), sin(i))*t) * amplitude;
	amplitude *= .35;
	lacunarity *= 2.52;
  }
  float w = height;
  float l = 1-dot(uv, uv);
  color = vec4(1.);
  color.a = smoothstep(.3, .7, w) * easeOut(l);
}
)glsl");
}

void drawSkyClouds(const Camera &camera, float time)
{
  glDepthMask(false); // do not write to depth buffer
  glm::mat4 M(1.f);
  // beware! if vertices go too far outside the clip range after the vertex shader
  // transformation, some may flicker, making triangles break and the whole plane
  // falling appart
  M = glm::translate(M, camera.getPosition() + glm::vec3{ 0, .3f, 0 });
  M = glm::scale(M, { 5.f, 1.f, 5.f });
  keepAliveResources->cloudsShader.bind();
  keepAliveResources->cloudsShader.setUniformMat4f("u_M", M);
  keepAliveResources->cloudsShader.setUniform2f("u_worldOffset", { camera.getPosition().x, camera.getPosition().z });
  keepAliveResources->cloudsShader.setUniform1f("u_time", time);
  keepAliveResources->cloudsShader.setUniformMat4f("u_VP", camera.getViewProjectionMatrix());
  keepAliveResources->planeMesh.draw();
  glDepthMask(true);
}

}