#include "Water.h"
#include <glad/glad.h>

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


	// Il y a un pb quelque part, je ne sais pas pk les textures calculées n'ont que la moitié de la skybox.
	// J'en fais appel à albin.

	// ok c'est pire que ca, la fonction ne marche pas dutout // à moitié

	// UPDATE : For some reasons, la texture de reflection (la première à etre calculée) a un depth_test POURRI
	//			mais pas la réfraction (la deuxieme).
	//	J'ai beau activer le depth test, mask, ou clear le depth bit, rien n'y fait

  const WaterSource &m_source = *m_sources.at(0);

  glEnable(GL_CLIP_DISTANCE0); // L'eau ne marchait plus à cause de ca qui avait disparu
	  


  // ~~~~~~~~~~~~ REFLEXION ~~~~~~~~~~~~ //

  Renderer::getStandardMeshShader().bind();
  Renderer::getStandardMeshShader().setUniform4f("u_plane", glm::vec4(0, 1, 0, -m_source.getHeight())); // clipping plane stuff
  Renderer::getStandardMeshShader().unbind();

  // place camera
  float distance = (camera.getPosition().y - m_source.getHeight()) * 2;

  camera.moveCamera({ 0, -distance, 0 });
  camera.inversePitch();
  camera.recalculateViewMatrix();
  camera.recalculateViewProjectionMatrix();

  // Take photo
  m_renderer.bindReflectionBuffer();
  renderFn();
  m_renderer.unbind();

  // undo camera setup

  camera.inversePitch();
  camera.moveCamera({ 0, distance, 0 });
  camera.recalculateViewMatrix();
  camera.recalculateViewProjectionMatrix();

  // ~~~~~~~~~~~~ REFRACTION ~~~~~~~~~~~~ //

  Renderer::getStandardMeshShader().bind();
  Renderer::getStandardMeshShader().setUniform4f("u_plane", glm::vec4(0, -1, 0, m_source.getHeight()));
  Renderer::getStandardMeshShader().unbind();

  m_renderer.bindRefractionBuffer();
  renderFn();
  m_renderer.unbind();

  glDisable(GL_CLIP_DISTANCE0);
  
  renderFn();
  m_renderer.onRenderWater(m_sources, camera);
  m_renderer.showDebugTextures();
}
