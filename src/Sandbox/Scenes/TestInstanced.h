#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "../Scene.h"
#include "../../abstraction/Renderer.h"
#include "../../abstraction/Cubemap.h"
#include "../../abstraction/UnifiedRenderer.h"
#include "../../World/Player.h"

namespace Renderer {

class InstancedMesh {
public:
  struct ModelVertex {
    glm::vec3 position; // model position
  };
  struct InstanceData {
    glm::vec3 position; // world position
    float height;       // blade height
  };

private:
  VertexBufferObject m_modelVBO, m_instanceVBO;
  IndexBufferObject m_ibo;
  VertexArray m_vao;
  unsigned int m_instanceCount;

public:
  InstancedMesh(const std::vector<ModelVertex> &vertices, const std::vector<unsigned int> &indices, const std::vector<InstanceData> &instances)
    : m_modelVBO(vertices.data(), vertices.size() * sizeof(ModelVertex)),
    m_instanceVBO(instances.data(), instances.size() * sizeof(InstanceData)),
    m_ibo(indices.data(), indices.size()),
    m_instanceCount(instances.size()),
    m_vao()
  {
    VertexBufferLayout modelLayout;
    VertexBufferLayout instanceLayout;
    modelLayout.push<float>(3); // position
    instanceLayout.push<float>(3); // position
    instanceLayout.push<float>(1); // height
    m_vao.addBuffer(m_modelVBO, modelLayout, m_ibo);
    m_vao.addInstanceBuffer(m_instanceVBO, instanceLayout, modelLayout);
    m_vao.unbind();
  }

  void draw() const
  {
    m_vao.bind();
    glDrawElementsInstanced(GL_TRIANGLES, m_ibo.getCount(), GL_UNSIGNED_INT, nullptr, m_instanceCount);
  }
};

}

class TestInstancedScene : public Scene {
private:
  Renderer::Cubemap   m_skybox;
  Player              m_player;
  Renderer::Mesh      m_groundMesh;
  Renderer::Texture   m_texture1;
  Renderer::Shader    m_grassShader;
  Renderer::InstancedMesh m_grassMesh;
public:
  TestInstancedScene()
    : m_skybox{
      "res/skybox_dbg/skybox_front.bmp", "res/skybox_dbg/skybox_back.bmp",
      "res/skybox_dbg/skybox_left.bmp",  "res/skybox_dbg/skybox_right.bmp",
      "res/skybox_dbg/skybox_top.bmp",   "res/skybox_dbg/skybox_bottom.bmp" },
      m_groundMesh(Renderer::createPlaneMesh()),
      m_texture1("res/textures/rock.jpg"),
      m_player(),

      m_grassMesh({ { {0,0,0} }, { {0,1,0} }, { {.1f,0,0} } },
                  { 0,1,2 },
                  /*{ { {0,0,0} }, { {1,0,0} }, { {0,0,1} } }*/ generateBladesInstances())
  {
    m_grassShader = Renderer::Shader(R"glsl(
#version 330 core

layout(location = 0) in vec3 im_position;
layout(location = 1) in vec3 ii_position;
layout(location = 2) in float ii_height;

out float o_fragmentHeight;

uniform mat4 u_VP;
uniform mat2 u_R;

void main()
{
  o_fragmentHeight = im_position.y;
  vec4 vertex = vec4(im_position, 1);
  vertex.xz = u_R * vertex.xz;
  vertex.y *= ii_height;
  vertex.xyz += ii_position;
  vertex = u_VP * vertex;
  gl_Position = vertex;
}

)glsl", R"glsl(
#version 330 core

out vec4 color;

in float o_fragmentHeight;

void main()
{
  color = vec4(vec3(o_fragmentHeight), 1);
}
)glsl");
  }

  static std::vector<Renderer::InstancedMesh::InstanceData> generateBladesInstances()
  {
    std::vector<Renderer::InstancedMesh::InstanceData> instances;
    for (int i = 0; i < 500; i++)
      for (int j = 0; j < 500; j++)
        instances.push_back({ { i/10.f + (i%3)/5.f, (i + j)/100.f, j/10.f + (i % 3) / 5.f }, ((i+3*j)%7)*.7f+.2f });
    return instances;
  }

  void step(float delta) override
  {
    m_player.step(delta);
  }

  void onRender() override
  {
    Renderer::Renderer::clear();
    Renderer::CubemapRenderer::drawCubemap(m_skybox, m_player.getCamera());
    m_texture1.bind();
    Renderer::renderMesh({ 0, 0, 0 }, { 10.f, 10.f, 10.f }, m_groundMesh, m_player.getCamera());


    float theta = 3.14f-m_player.getCamera().getYaw();
    float c = cos(theta), s = sin(theta);
    glm::mat2 facingCameraRotationMatrix = glm::mat2(c, s, -s, c);

    m_grassShader.bind();
    m_grassShader.setUniformMat4f("u_VP", m_player.getCamera().getViewProjectionMatrix());
    m_grassShader.setUniformMat2f("u_R", facingCameraRotationMatrix);
    m_grassMesh.draw();
  }

  void onImGuiRender() override
  {
  }
};