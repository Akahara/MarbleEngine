#include "WaterRenderer.h"

#include <glad/glad.h>

WaterRenderer::WaterRenderer() {


	m_waterReflectionFbo.setTargetTexture(m_reflectionTexture);

	m_waterRefractionFbo.setTargetTexture(m_refractionTexture);
	m_waterRefractionFbo.setDepthTexture(m_depthTexture);

}

void WaterRenderer::updateMoveFactor(float deltaTime)
{
  m_moveFactor += WAVE_SPEED * deltaTime;
  m_moveFactor = std::fmod(m_moveFactor, 1.f);
  m_waterShader.bind();
  m_waterShader.setUniform1f("u_moveFactor", m_moveFactor);
}

void WaterRenderer::setupCameraForReflection(Renderer::Camera &cam, const WaterSource &source)
{
  // compute distance to ground
  float distance = (cam.getPosition().y - source.getHeight()) * 2;

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
  float distance = (source.getHeight() - cam.getPosition().y) * 2;

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
  Renderer::FrameBufferObject::setViewportToTexture(m_reflectionTexture);
}

void WaterRenderer::bindRefractionBuffer()
{
  m_waterRefractionFbo.bind();
  Renderer::FrameBufferObject::setViewportToTexture(m_refractionTexture);
}

void WaterRenderer::unbind()
{
  Renderer::FrameBufferObject::unbind();
}

void WaterRenderer::onRenderWater(const std::vector<WaterSource *> &waterSources, const Renderer::Camera &camera)
{
  m_waterShader.bind();
  m_reflectionTexture.bind(1);
  m_refractionTexture.bind(2);
  m_dudvTexture.bind(3);
  m_normalTexture.bind(4);
  m_depthTexture.bind(5);

  m_waterShader.setUniform3f("u_camPos", camera.getPosition());
  m_waterShader.setUniform1i("u_ReflectionTexture", 1);
  m_waterShader.setUniform1i("u_RefractionTexture", 2);
  m_waterShader.setUniform1i("u_dudvMap", 3);
  m_waterShader.setUniform1i("u_normalMap", 4);
  m_waterShader.setUniform1i("u_depthMap", 5);


  for (const WaterSource *source : waterSources) {
	drawWaterSource(*source, camera);
  }


}

void WaterRenderer::drawWaterSource(const WaterSource &source, const Renderer::Camera &camera)
{
  glm::mat4 M(1.f);
  M = glm::translate(M, { source.getPosition().x, source.getHeight(), source.getPosition().y });
  M = glm::scale(M, glm::vec3(source.getSize()));

  m_waterShader.bind();
  m_waterShader.setUniformMat4f("u_M", M);
  m_waterShader.setUniformMat4f("u_VP", camera.getViewProjectionMatrix());
  source.draw();
  m_waterShader.unbind();
}

void WaterRenderer::showDebugTextures()
{

  Renderer::renderDebugGUIQuadWithTexture(m_reflectionTexture, { -0.75, -0.75 }, { 0.5, 0.5 });
  Renderer::renderDebugGUIQuadWithTexture(m_refractionTexture, { 0.25, -0.75 }, { 0.5, 0.5 });

}
