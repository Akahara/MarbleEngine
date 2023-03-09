#include "Water.h"
#include <glad/glad.h>

void World::Water::onRender(const std::function<void()> &renderFn, Renderer::Camera &camera)
{
  const WaterSource &m_source = m_sources.at(0);

  // ~~~~~~~~~~~~ REFLEXION ~~~~~~~~~~~~ //

  Renderer::getStandardMeshShader()->bind();
  Renderer::getStandardMeshShader()->setUniform4f("u_plane", glm::vec4(0, 1, 0, -m_source.position.y)); // clipping plane stuff
  Renderer::Shader::unbind();

  // place camera
  float distance = (camera.getPosition().y - m_source.position.y) * 2;

  camera.moveCamera({ 0, -distance, 0 });
  camera.inversePitch();
  camera.recalculateViewMatrix();
  camera.recalculateViewProjectionMatrix();

  // Take photo

  glEnable(GL_CLIP_DISTANCE0);

  m_renderer.bindReflectionBuffer();
  Renderer::clear();
  renderFn();
  m_renderer.unbind();

  // undo camera setup

  camera.inversePitch();
  camera.moveCamera({ 0, distance, 0 });
  camera.recalculateViewMatrix();
  camera.recalculateViewProjectionMatrix();

  // ~~~~~~~~~~~~ REFRACTION ~~~~~~~~~~~~ //

  Renderer::getStandardMeshShader()->bind();
  Renderer::getStandardMeshShader()->setUniform4f("u_plane", glm::vec4(0, -1, 0, m_source.position.y));
  Renderer::Shader::unbind();

  m_renderer.bindRefractionBuffer();
  Renderer::clear();
  renderFn();
  m_renderer.unbind();

  glDisable(GL_CLIP_DISTANCE0);
  
  renderFn();
  m_renderer.onRenderWater(m_sources, camera);
}
