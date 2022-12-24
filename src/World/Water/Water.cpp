#include "Water.h"

void World::Water::addSource(float level, const glm::vec2 &pos, float size)
{
  WaterSource *source = new WaterSource{ level, pos };
  source->setSize(size);
  m_sources.push_back(source);
}

void World::Water::removeSource(int index)
{
  delete m_sources.at(index);
  m_sources.erase(m_sources.begin() + index);
}

void World::Water::onRender(const std::function<void()> &renderFn, Renderer::Camera &camera)
{
  const WaterSource &m_source = *m_sources.at(0);

  Renderer::getStandardMeshShader().bind();
  Renderer::getStandardMeshShader().setUniform4f("u_plane", glm::vec4(0, 1, 0, -m_source.getHeight())); // clipping plane stuff
  Renderer::getStandardMeshShader().unbind();


  float distance = (camera.getPosition().y - m_source.getHeight()) * 2;

  // place camera
  camera.moveCamera({ 0, -distance, 0 });
  camera.inversePitch();

  camera.recalculateViewMatrix();
  camera.recalculateViewProjectionMatrix();


  m_renderer.bindReflectionBuffer();
  renderFn();

  // Change view

  camera.moveCamera({ 0, distance, 0 });
  camera.inversePitch();
  camera.recalculateViewMatrix();
  camera.recalculateViewProjectionMatrix();
  m_renderer.unbind();

  // ---

  // Refraction

  m_renderer.bindRefractionBuffer();

  Renderer::getStandardMeshShader().bind();
  Renderer::getStandardMeshShader().setUniform4f("u_plane", glm::vec4(0, -1, 0, m_source.getHeight()));
  Renderer::getStandardMeshShader().unbind();

  renderFn();
  m_renderer.unbind();


  //m_waterRenderer.writeTexture();
  renderFn();
  m_renderer.onRenderWater(m_sources, camera);
}
