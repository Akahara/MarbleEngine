#include "Sky.h"

namespace World {

Sky::Sky()
  : m_skybox("res/skybox_dbg/skybox_front.bmp", "res/skybox_dbg/skybox_back.bmp",
             "res/skybox_dbg/skybox_left.bmp", "res/skybox_dbg/skybox_right.bmp",
             "res/skybox_dbg/skybox_top.bmp", "res/skybox_dbg/skybox_bottom.bmp")
{

}

void Sky::Step(float delta)
{
}

void Sky::Render(const Renderer::Camera &camera, glm::vec3 playerPosition) const
{
  Renderer::CubemapRenderer::DrawCubemap(m_skybox, camera, playerPosition);
}

}
