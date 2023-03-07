#pragma once

#include <vector>

#include "../../abstraction/Texture.h"
#include "../../abstraction/UnifiedRenderer.h"
#include "../../abstraction/FrameBufferObject.h"
#include "../../abstraction/Camera.h"

namespace World {

struct WaterSource {
  glm::vec3 position;
  glm::vec2 size;
};

class WaterRenderer
{
private:
  static constexpr float WAVE_SPEED = 0.05f;

  std::shared_ptr<Renderer::Texture> m_dudvTexture;   // distortion
  std::shared_ptr<Renderer::Texture> m_normalTexture; // normal
  std::shared_ptr<Renderer::Shader> m_waterShader;

  Renderer::FrameBufferObject m_waterRefractionFbo;
  Renderer::FrameBufferObject m_waterReflectionFbo;

  std::shared_ptr<Renderer::Texture> m_refractionTexture; // what u see under water
  std::shared_ptr<Renderer::Texture> m_reflectionTexture; // what is reflected
  std::shared_ptr<Renderer::Texture> m_depthTexture;

  Renderer::Mesh m_planeMesh;

  float m_moveFactor = 0;

public:
  WaterRenderer();

  void updateMoveFactor(float deltaTime);

  // -------- Reflection

  void setupCameraForReflection(Renderer::Camera &cam, const WaterSource &source);
  void undoSetupCameraForReflection(Renderer::Camera &cam, const WaterSource &source);

  void bindReflectionBuffer();

  //--------- Refraction

  void bindRefractionBuffer();

  void unbind();

  /* TO BE CALLED AFTER BINDING THE VFX PIPELINE AND THE SCENE HAS BEEN RENDERED IN THE WATER FBO */
  void onRenderWater(const std::vector<WaterSource> &waterSources, const Renderer::Camera &camera);

  // Debug method, writes the rendered reflection and refraction textures to files
  void showDebugTextures();
};

}