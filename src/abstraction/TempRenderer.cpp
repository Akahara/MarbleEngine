#include "TempRenderer.h"

#include "Shader.h"
#include "VertexArray.h"
#include "IndexBufferObject.h"

#include <memory>
#include <iostream>

using namespace Renderer;

namespace TempRenderer {

    static std::unique_ptr<VertexBufferObject> vbo;
    static  std::unique_ptr<IndexBufferObject> ibo;
    static  std::unique_ptr<VertexArray> vao;

    static std::unique_ptr<VertexBufferObject> vbo2;
    static  std::unique_ptr<IndexBufferObject> ibo2;
    static  std::unique_ptr<VertexArray> vao2;


static struct renderData {
  Shader *shader;
  VertexArray *vao;
} cmRenderData;

static struct KAR {
  VertexBufferObject *vbo;
  IndexBufferObject *ibo;
} keepAliveResources;

void Init()
{
  cmRenderData.shader = new Shader{
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

  keepAliveResources.vbo = new VertexBufferObject(vertices, sizeof(vertices));
  keepAliveResources.ibo = new IndexBufferObject(indices, sizeof(indices) / sizeof(indices[0]));

  VertexBufferLayout layout;
  layout.push<float>(3);
  cmRenderData.vao = new VertexArray;
  cmRenderData.vao->addBuffer(*keepAliveResources.vbo, layout);
  cmRenderData.vao->Unbind();





}

void RenderCube(glm::vec3 position, glm::vec3 size, glm::vec3 color, const glm::mat4 &VP)
{
  glm::mat4 M(1.f);
  M = glm::translate(M, position);
  M = glm::scale(M, size);
  cmRenderData.shader->Bind();
  cmRenderData.vao->Bind();
  cmRenderData.shader->SetUniformMat4f("u_VP", VP);
  cmRenderData.shader->SetUniformMat4f("u_M", M);
  cmRenderData.shader->SetUniform4f("u_color", color.r, color.g, color.b, 1.f);
  keepAliveResources.ibo->Bind();

  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
}


void RenderPlane(const glm::vec3& position, const glm::vec3& size, const glm::vec3& color, float rotation, const glm::mat4& VP, bool drawLines) {


    std::cout << "========================== PLANE ===================================" << std::endl;
    float verticesPlane[] = {
      -1, -1, 0,
      +1, -1, 0,
      +1, +1, 0,
      -1, +1, 0

    };

    /*
    0 0.5 0,
    0 0.8 0,
    0 1.1 0,
    0.3 1.1 0
*/

    unsigned int indicesPlane[] = {
      3,2,0,
      2,1,0,

    };

    
    vbo = std::make_unique<VertexBufferObject>(verticesPlane, sizeof(verticesPlane));
    ibo = std::make_unique <IndexBufferObject>(indicesPlane, sizeof(indicesPlane) / sizeof(indicesPlane[0]));

    VertexBufferLayout layout;
    layout.push<float>(3);
    vao = std::make_unique<VertexArray>();
    vao->addBuffer(*vbo, layout);
    vao->Unbind();

    glm::mat4 M(1.f);
    M = glm::translate(M, position);
    M = glm::scale(M, size);
    M = glm::rotate(M, rotation, glm::vec3{ 1,0,0 });

    cmRenderData.shader->Bind();
    vao->Bind();

    cmRenderData.shader->SetUniformMat4f("u_VP", VP);
    cmRenderData.shader->SetUniformMat4f("u_M", M);
    cmRenderData.shader->SetUniform4f("u_color", color.r, color.g, color.b, 1.f);
    ibo->Bind();

    if (drawLines) {

        glDrawElements(GL_LINES, 6, GL_UNSIGNED_INT, nullptr);

    }
    else {
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    }
    ibo->Unbind();

}
void RenderGrid(const glm::vec3& position, float quadSize, int quadsPerSide, const glm::vec3& color, const glm::mat4& VP, bool drawLines) 

{

    std::cout << "\ngrid ==================== \n";
    unsigned int nbOfQuads = glm::pow(quadsPerSide, 2);

    // Create vertices              // nb of verticies              // size of a vertex
    float* verticesGrid = new float[glm::pow(quadsPerSide+2, 2)      * 3];

    // Calculate the step for each vertex
    float step = quadSize / quadsPerSide;

    int x = 0;
    int y = 0;
    int test = 0;
    for (int i = 0; i < glm::pow(quadsPerSide + 1, 2) * 3; i += 3) {


        verticesGrid[i] = position.x + x * step;                  // x
        verticesGrid[i + 1] = position.y + y * step;              // y
        verticesGrid[i + 2] = 0.f;


        x++;
        test++;
        if (x == nbOfQuads + 1) {
            y++; 
            x = 0;
        }
    }

    float verticesPlane[] = {
      -1, -1, 0,
      +1, -1, 0,
      -1, +1, 0
      +1, +1, 0,

    };


    for (int i = 0; i < glm::pow(quadsPerSide + 1, 2) * 3; i++)
    {
        
        if (i % 3 == 0) std::cout << std::endl;
        std::cout << verticesGrid[i] << " ";

    }
    

    // Calculate the indices

    // We have 6 indices per quad and count^2 quads
    unsigned int* indicesGrid = new unsigned int[nbOfQuads * 6];



    /*
    
             x4 _________________ x3
                |               |
                |               |
                |               |
                |               |
                |               |
                |_______________|
             x1                   x2


    Because we fill the vertex array from bottom to top, left to right, we can deduce that :

    indice(x2) = indice(x1 + 1);
    indice(x3) = indice(x1 + quadsPerSide + 1);
    indice(x4) = indice(x1 + (quadsPerSide + 1) + 1);

    And we fill the indices array as so (for a single quad) :

    { 
        x4, x3, x1,
        x3, x2, x1
    }

    std::cout << "number of quads : " << nbOfQuads << std::endl;

    */

    for (int i = 0; i < nbOfQuads * 6; i += 6) {

        /*
        indicesGrid[i]      = i + quadsPerSide + 2;
        indicesGrid[i+1]    = i + quadsPerSide + 1;
        indicesGrid[i+2]    = i ;
        indicesGrid[i+3]    = i + quadsPerSide + 2;
        indicesGrid[i+4]    = i+1;
        indicesGrid[i+5]    = i;
        */
        indicesGrid[i] = i;
        indicesGrid[i + 1] = i + 1;
        indicesGrid[i + 2] = i + quadsPerSide + 1;
        indicesGrid[i + 3] = i + quadsPerSide + 1;
        indicesGrid[i + 4] = i + quadsPerSide + 2;
        indicesGrid[i + 5] = i;
    }

    for (int i = 0; i < nbOfQuads * 6; i++) {

        if (i % 6 == 0) {
            std::cout << std::endl;
        }
        std::cout << indicesGrid[i] << " ";

    }

    // render

    vbo2 = std::make_unique<VertexBufferObject>(verticesPlane, sizeof(verticesPlane));
    ibo2 = std::make_unique <IndexBufferObject>(indicesGrid, nbOfQuads * 6);

    VertexBufferLayout layout;
    layout.push<float>(3);
    vao2 = std::make_unique<VertexArray>();
    vao2->addBuffer(*vbo2, layout);
    vao2->Unbind();

    glm::mat4 M(1.f);
    M = glm::translate(M, position);
    M = glm::scale(M, { quadSize,quadSize,quadSize });

    cmRenderData.shader->Bind();
    vao2->Bind();

    cmRenderData.shader->SetUniformMat4f("u_VP", VP);
    cmRenderData.shader->SetUniformMat4f("u_M", M);
    cmRenderData.shader->SetUniform4f("u_color", color.r, color.g, color.b, 1.f);
    ibo2->Bind();

    if (drawLines) {

        glDrawElements(GL_LINES, nbOfQuads * 6, GL_UNSIGNED_INT, nullptr);

    }
    else {
        glDrawElements(GL_TRIANGLES, nbOfQuads * 6, GL_UNSIGNED_INT, nullptr);
    }

    ibo2->Unbind();


}   


}