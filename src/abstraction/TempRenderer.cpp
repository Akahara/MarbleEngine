#include "TempRenderer.h"

#include "Shader.h"
#include "VertexArray.h"
#include "IndexBufferObject.h"

#include "Mesh.h"

#include <memory>
#include <iostream>

using namespace Renderer;

namespace TempRenderer {

static std::unique_ptr<VertexBufferObject> vbo;
static std::unique_ptr<IndexBufferObject> ibo;
static std::unique_ptr<VertexArray> vao;

static std::unique_ptr<VertexBufferObject> vbo2;
static std::unique_ptr<IndexBufferObject> ibo2;
static std::unique_ptr<VertexArray> vao2;

static struct CmRenderData {
  Shader shader;
  VertexArray vao;
} *cmRenderData;

static struct KeepAliveResources {
  VertexBufferObject vbo;
  IndexBufferObject ibo;
} *keepAliveResources;

struct VertexTemp { // TODO use Renderer::Vertex instead of this struct
  glm::vec3 position;
  glm::vec2 uv;
};


void Init()
{
  keepAliveResources = new KeepAliveResources;
  cmRenderData = new CmRenderData;

  cmRenderData->shader = Shader{
 R"glsl(
#version 330 core

layout(location = 0) in vec3 i_position;
layout(location = 1) in vec2 i_uv;


out vec2 o_uv;
out float o_height;
uniform mat4 u_VP;
uniform mat4 u_M;


void main()
{
   o_uv = i_uv;
o_height = i_position.y;
  gl_Position = u_VP * u_M * vec4(i_position, +1.0);
}
)glsl", R"glsl(
#version 330 core

in float o_height;
out vec4 color;
in vec2 o_uv;
uniform vec4 u_color;
uniform sampler2D u_Texture2D;

void main()
{

if (o_height < 0.3) { color.rgb = vec3(0.05,0.11,0.58); }
else if (o_height < 0.4) {color.rgb = vec3(0.17,0.5,0.78);}
else if (o_height < 0.45) {color.rgb = vec3(0.75,0.68,0.28);}
else if (o_height < 0.55) {color.rgb = vec3(0.1,0.4,0.04);}
else if (o_height < 0.6) {color.rgb = vec3(0.07,0.22,0.03);}
else if (o_height < 0.7) {color.rgb = vec3(0.3,0.21,0.1);}
else if (o_height < 0.9) {color.rgb = vec3(0.13,0.05,0.0);}
else { color.rgb = vec3(0.86,0.86,0.86); }
color -= (texture(u_Texture2D, o_uv+vec2(.002, 0)) - texture(u_Texture2D, o_uv)) *6;
color.a = 1.0F;

//color = texture(u_Texture2D, o_uv);
}
)glsl" };

  float vertices[] = {
    -.5f, -.5f, -.5f,
    +.5f, -.5f, -.5f,
    +.5f, +.5f, -.5f,
    -.5f, +.5f, -.5f,
    -.5f, -.5f, +.5f,
    +.5f, -.5f, +.5f,
    +.5f, +.5f, +.5f,
    -.5f, +.5f, +.5f,
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
  cmRenderData->vao.addBuffer(keepAliveResources->vbo, layout, keepAliveResources->ibo);
  cmRenderData->vao.Unbind();
}

void RenderCube(glm::vec3 position, glm::vec3 size, glm::vec3 color, const glm::mat4 &VP)
{
  glm::mat4 M(1.f);
  M = glm::translate(M, position);
  M = glm::scale(M, size);
  cmRenderData->vao.Bind();
  cmRenderData->shader.Bind();
  cmRenderData->shader.SetUniformMat4f("u_VP", VP);
  cmRenderData->shader.SetUniformMat4f("u_M", M);
  cmRenderData->shader.SetUniform4f("u_color", color.r, color.g, color.b, 1.f);

  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
}

void RenderMesh(glm::vec3 position, glm::vec3 size, const Mesh &mesh, const glm::mat4 &VP)
{
  glm::mat4 M(1.f);
  M = glm::translate(M, position);
  M = glm::scale(M, size);
  cmRenderData->shader.Bind();
  cmRenderData->shader.SetUniformMat4f("u_M", M);
  cmRenderData->shader.SetUniformMat4f("u_VP", VP);
  mesh.Draw();
}


void RenderPlane(const glm::vec3& position, const glm::vec3& size, const glm::vec3& color, float rotation, const glm::mat4& VP, bool drawLines) {


    std::cout << "========================== PLANE ===================================" << std::endl;
    float verticesPlane[] = {
      -1, -1, 0,
      +1, -1, 0,
      +1, +1, 0,
      -1, +1, 0

    };


    unsigned int indicesPlane[] = {
      3,2,0,
      2,1,0,

    };

    
    vbo = std::make_unique<VertexBufferObject>(verticesPlane, sizeof(verticesPlane));
    ibo = std::make_unique<IndexBufferObject>(indicesPlane, sizeof(indicesPlane) / sizeof(indicesPlane[0]));

    VertexBufferLayout layout;
    layout.push<float>(3);
    vao = std::make_unique<VertexArray>();
    vao->addBuffer(*vbo, layout, *ibo);
    vao->Unbind();

    glm::mat4 M(1.f);
    M = glm::translate(M, position);
    M = glm::scale(M, size);
    M = glm::rotate(M, rotation, glm::vec3{ 1,0,0 });

    cmRenderData->shader.Bind();
    vao->Bind();

    cmRenderData->shader.SetUniformMat4f("u_VP", VP);
    cmRenderData->shader.SetUniformMat4f("u_M", M);
    cmRenderData->shader.SetUniform4f("u_color", color.r, color.g, color.b, 1.f);

    if (drawLines) {
        glDrawElements(GL_LINES, 6, GL_UNSIGNED_INT, nullptr);
    } else {
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    }

    vao->Unbind();
}

}