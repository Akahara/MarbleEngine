#include "WaterRenderer.h"

#include <glad/glad.h>

#include "../../abstraction/Window.h"

using Renderer::Texture;

namespace World {

// required textures for water rendering
enum TextureSlot : unsigned int {
  REFLECTION,
  REFRACTION,
  DUDV,
  NORMAL,
  DEPTH,
};

WaterRenderer::WaterRenderer()
  : m_waterShader(Renderer::loadShaderFromFiles("res/shaders/standard.vs", "res/shaders/water.fs")),
  m_depthTexture(std::make_shared<Texture>(Texture::createDepthTexture(Window::getWinWidth(), Window::getWinHeight()))),
  m_refractionTexture(std::make_shared<Texture>(Window::getWinWidth(), Window::getWinHeight())),
  m_reflectionTexture(std::make_shared<Texture>(Window::getWinWidth(), Window::getWinHeight())),
  m_dudvTexture(std::make_shared<Texture>("res/textures/dudvWater.png")),
  m_normalTexture(std::make_shared<Texture>("res/textures/waterNormal.png")),
  m_planeMesh(Renderer::createPlaneModel(), std::make_shared<Renderer::Material>())
{
  auto &material = m_planeMesh.getMaterial();
  m_waterReflectionFbo.setTargetTexture(*m_reflectionTexture);
  m_waterReflectionFbo.setDepthTexture(*m_depthTexture);
  m_waterRefractionFbo.setTargetTexture(*m_refractionTexture);
  m_waterRefractionFbo.setDepthTexture(*m_depthTexture);
  m_waterShader->bind();
  m_waterShader->setUniform1i("u_ReflectionTexture", TextureSlot::REFLECTION);
  m_waterShader->setUniform1i("u_RefractionTexture", TextureSlot::REFRACTION);
  m_waterShader->setUniform1i("u_dudvMap",   TextureSlot::DUDV);
  m_waterShader->setUniform1i("u_normalMap", TextureSlot::NORMAL);
  m_waterShader->setUniform1i("u_depthMap",  TextureSlot::DEPTH);
  material->shader = m_waterShader;
  material->textures[TextureSlot::REFLECTION] = m_reflectionTexture;
  material->textures[TextureSlot::REFRACTION] = m_refractionTexture;
  material->textures[TextureSlot::DUDV]   = m_reflectionTexture;
  material->textures[TextureSlot::NORMAL] = m_normalTexture;
  material->textures[TextureSlot::DEPTH]  = m_depthTexture;
}

void WaterRenderer::updateMoveFactor(float deltaTime)
{
  m_moveFactor += WAVE_SPEED * deltaTime;
  m_moveFactor = std::fmod(m_moveFactor, 1.f);
  m_waterShader->bind();
  m_waterShader->setUniform1f("u_moveFactor", m_moveFactor);
}

void WaterRenderer::setupCameraForReflection(Renderer::Camera &cam, const WaterSource &source)
{
  // compute distance to ground
  float distance = (cam.getPosition().y - source.position.y) * 2;

  // place camera
  glm::vec3 camPosition = cam.getPosition();
  camPosition.y -= distance;
  cam.setPosition(camPosition);
  // Change view
  cam.inversePitch();
}

void WaterRenderer::undoSetupCameraForReflection(Renderer::Camera &cam, const WaterSource &source)
{
  // compute distance to ground
  float distance = (source.position.y - cam.getPosition().y) * 2;

  // place camera
  glm::vec3 camPosition = cam.getPosition();
  camPosition.y += distance;
  cam.setPosition(camPosition);

  // Change view
  cam.inversePitch();
}

void WaterRenderer::bindReflectionBuffer()
{
  m_waterReflectionFbo.bind();
  Renderer::FrameBufferObject::setViewportToTexture(*m_reflectionTexture);
}

void WaterRenderer::bindRefractionBuffer()
{
  m_waterRefractionFbo.bind();
  Renderer::FrameBufferObject::setViewportToTexture(*m_refractionTexture);
}

void WaterRenderer::unbind()
{
  Renderer::FrameBufferObject::unbind();
}

void WaterRenderer::onRenderWater(const std::vector<WaterSource> &waterSources, const Renderer::Camera &camera)
{
  m_waterShader->bind();
  m_waterShader->setUniform3f("u_camPos", camera.getPosition());

  for (const WaterSource &source : waterSources) {
    m_planeMesh.getTransform().position = source.position;
    m_planeMesh.getTransform().scale = { source.size.x, 1, source.size.y };
	Renderer::renderMesh(camera, m_planeMesh);
  }
}

void WaterRenderer::showDebugTextures()
{
  Renderer::renderDebugGUIQuadWithTexture(*m_reflectionTexture, { -0.75, -0.75 }, { 0.5, 0.5 });
  Renderer::renderDebugGUIQuadWithTexture(*m_refractionTexture, { 0.25, -0.75 }, { 0.5, 0.5 });
}

}