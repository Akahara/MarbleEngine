#include "TempRenderer.h"

#include "Shader.h"
#include "VertexArray.h"
#include "IndexBufferObject.h"

using namespace Renderer;

namespace TempRenderer {

static struct CmRenderData {
  Shader shader;
  VertexArray vao;
} *cmRenderData;

static struct KeepAliveResources {
  VertexBufferObject vbo;
  IndexBufferObject ibo;
} *keepAliveResources;

void Init()
{
  keepAliveResources = new KeepAliveResources;
  cmRenderData = new CmRenderData;

  cmRenderData->shader = Shader{
 R"glsl(
#version 330 core

layout(location = 0) in vec3 i_position;

uniform mat4 u_VP;
uniform mat4 u_M;

void main()
{
  gl_Position = u_VP * u_M * vec4(i_position, +1.0);
}
)glsl", R"glsl(
#version 330 core

out vec4 color;

uniform vec4 u_color;

void main()
{    
  color = u_color;
}
)glsl" };

  float vertices[] = {
    -1, -1, -1,
    +1, -1, -1,
    +1, +1, -1,
    -1, +1, -1,
    -1, -1, +1,
    +1, -1, +1,
    +1, +1, +1,
    -1, +1, +1,
  };

  unsigned int indices[] = {
    0, 3, 1, 1, 3, 2,
    1, 2, 5, 5, 2, 6,
    5, 6, 4, 4, 6, 7,
    4, 7, 0, 0, 7, 3,
    3, 7, 2, 2, 7, 6,
    4, 0, 5, 5, 0, 1
  };

  keepAliveResources->vbo = VertexBufferObject(vertices, sizeof(vertices));
  keepAliveResources->ibo = IndexBufferObject(indices, sizeof(indices) / sizeof(indices[0]));

  VertexBufferLayout layout;
  layout.push<float>(3);
  cmRenderData->vao.addBuffer(keepAliveResources->vbo, layout);
  cmRenderData->vao.Unbind();
}

void RenderCube(glm::vec3 position, glm::vec3 size, glm::vec3 color, const glm::mat4 &VP)
{
  glm::mat4 M(1.f);
  M = glm::translate(M, position);
  M = glm::scale(M, size);
  cmRenderData->shader.Bind();
  cmRenderData->vao.Bind();
  cmRenderData->shader.SetUniformMat4f("u_VP", VP);
  cmRenderData->shader.SetUniformMat4f("u_M", M);
  cmRenderData->shader.SetUniform4f("u_color", color.r, color.g, color.b, 1.f);
  keepAliveResources->ibo.Bind();

  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
}

}