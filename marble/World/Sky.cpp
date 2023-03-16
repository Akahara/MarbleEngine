#include "Sky.h"

#include <glm/glm.hpp>
#include <glad/glad.h>
#include "../vendor/imgui/imgui.h"

#include "../abstraction/Shader.h"
#include "../abstraction/VertexBufferObject.h"
#include "../abstraction/IndexBufferObject.h"
#include "../abstraction/Shader.h"
#include "../abstraction/Cubemap.h"
#include "../World/Player.h"
#include "../Utils/Mathf.h"
#include "../abstraction/UnifiedRenderer.h"

namespace World {

Sky::Sky(const SkyboxesType& type)
{
  std::string EXTENSION = ".bmp";
	
  std::stringstream path;
  path << "res/skybox/";

  switch (type) {
  case SAND: path << "desert/";  break;
  case SNOW: path << "snow/";    break;
  default:   path << "default/"; break;
  }

  std::string finalPath = path.str();
  std::string filesName[] = {
	finalPath + "front"  + EXTENSION,
	finalPath + "back"	 + EXTENSION,
	finalPath + "left"	 + EXTENSION,
	finalPath + "right"  + EXTENSION,
	finalPath + "top"    + EXTENSION,
	finalPath + "bottom" + EXTENSION
  };

  m_skybox = std::make_unique<Renderer::Cubemap>(
	filesName[0], filesName[1], filesName[2],
	filesName[3], filesName[4], filesName[5]);
}

void Sky::render(const Renderer::Camera &camera, float time, bool withClouds) const
{
	Renderer::renderCubemap(camera, *m_skybox);
	if(withClouds) Renderer::SkyRenderer::drawSkyClouds(camera, time);
}

}


namespace Renderer::SkyRenderer {

static struct KeepAliveResources {
  Mesh planeMesh;
  std::shared_ptr<Shader> cloudsShader;
} *keepAliveResources;

void init()
{
  keepAliveResources = new KeepAliveResources;
  keepAliveResources->planeMesh = Mesh(Renderer::createPlaneModel(true), std::make_shared<Renderer::Material>());
  keepAliveResources->cloudsShader = Renderer::loadShaderFromFiles("res/shaders/clouds.vs", "res/shaders/clouds.fs");
  keepAliveResources->planeMesh.getMaterial()->shader = keepAliveResources->cloudsShader;
}

void drawSkyClouds(const Camera &camera, float time)
{
  glDepthMask(false); // do not write to depth buffer
  glDepthFunc(GL_LEQUAL);
  glm::mat4 M(1.f);
  // beware! if vertices go too far outside the clip range after the vertex shader
  // transformation, some may flicker, making triangles break and the whole plane
  // falling appart
  // // FIX sky renderer
  //M = glm::translate(M, camera.getPosition() + glm::vec3{ 0, .3f, 0 });
  //M = glm::scale(M, { 5.f, 1.f, 5.f });
  //keepAliveResources->cloudsShader.bind();
  //keepAliveResources->cloudsShader.setUniformMat4f("u_M", M);
  //keepAliveResources->cloudsShader.setUniformMat4f("u_VP", camera.getViewProjectionMatrix());
  //keepAliveResources->cloudsShader.setUniform2f("u_worldOffset", { camera.getPosition().x, camera.getPosition().z });
  //keepAliveResources->cloudsShader.setUniform1f("u_time", time);
  //keepAliveResources->planeMesh.draw();
  glDepthFunc(GL_LESS);
  glDepthMask(true);
}

}