#include "UnifiedRenderer.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <memory>
#include <cstring>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glad/glad.h>

#include "Window.h"
#include "Camera.h"
#include "Mesh.h"

#include "../World/Light/Light.h" // TODO move light.h to the abstraction package
                                  // abstraction should not depend on world, the inverse is possible

#ifndef WIN32
#include <sys/stat.h>
#endif

namespace Renderer {

static struct KeepAliveResources {
  Shader             standardMeshShader;
  Shader             standardLineShader;
  Shader             standardLightsShader;
  Shader             cubemapShader;
  Shader             debugNormalsShader;
  Shader             standardDepthPassShader;

  Shader             debugFlatScreenShader;

  Shader             debugCubeShader;
  Mesh               debugCubeMesh;
  VertexArray        cubemapVAO;
  VertexBufferObject cubemapVBO;
  IndexBufferObject  cubemapIBO;
  VertexArray        lineVAO;
  IndexBufferObject  lineIBO;
  VertexBufferObject emptyVBO; // used by the line vao
} *s_keepAliveResources = nullptr;

static struct State {
  Shader *activeStandardShader;
} s_state;


Shader loadShaderFromFiles(const fs::path &vertexPath, const fs::path &fragmentPath)
{
  std::ifstream vertexFile{ vertexPath };
  std::ifstream fragmentFile{ fragmentPath };
  std::stringstream buffer;
  buffer << vertexFile.rdbuf();
  std::string vertexCode = buffer.str();
  buffer.str("");
  buffer << fragmentFile.rdbuf();
  std::string fragmentCode = buffer.str();
  return Shader{ vertexCode, fragmentCode };
}

Mesh createCubeMesh()
{
  return loadMeshFromFile("res/meshes/cube.obj");
}

Mesh createPlaneMesh(bool facingDown)
{
  std::vector<Vertex> vertices{
    // position             uv            normal (up)         // color
    { { -.5f, 0.f, -.5f }, { 1.f, 1.f }, { 0, 1.f, 0 }, {1.0f, 1.0f, 0.0f}, },
    { { -.5f, 0.f, +.5f }, { 1.f, 0.f }, { 0, 1.f, 0 }, {1.0f, 1.0f, 0.0f}, },
    { { +.5f, 0.f, +.5f }, { 0.f, 0.f }, { 0, 1.f, 0 }, {1.0f, 1.0f, 0.0f}, },
    { { +.5f, 0.f, -.5f }, { 0.f, 1.f }, { 0, 1.f, 0 }, {1.0f, 1.0f, 0.0f}, },
  };
  using i = std::initializer_list<unsigned int>;
  std::vector<unsigned int> indices{ facingDown ? i{1,0,2, 3,2,0} : i{0,1,2, 2,3,0} };
  return Mesh(vertices, indices);
}

static void skipStreamText(std::istream &stream, const char *text)
{
  char c;
  while (*text) {
    if ((c = stream.get()) != *text) {
      std::cerr << "Expected " << *text << " but got " << c;
      throw std::runtime_error("Unexpected charactor got in stream");
    }
    text++;
  }
}

Mesh loadMeshFromFile(const fs::path& objPath)
{

    // TODO : read the MTL file, and try to find the correct filename for the materials
  std::ifstream modelFile{ objPath };
  constexpr size_t bufSize = 10000;
  char lineBuffer[bufSize];

  std::vector<glm::vec3> positions;
  std::vector<glm::vec3> normals;
  std::vector<glm::vec2> uvs;
  std::unordered_map<int, std::shared_ptr<Texture>> slotsTextures = {
      {0, std::make_shared<Texture>("res/textures/no_texture.png") }
  };

  std::unordered_map<std::string, std::string> cacheMatFile;
  std::vector<std::string> materials_slots;

  int currentTextureSlot = -1;
  std::string mtllib;

  positions.push_back({ 0,0,0 });
  normals.push_back({ 0,0,0 });
  uvs.push_back({ 0,0 });

  uvs.emplace_back();


  std::vector<std::tuple<int, int, int>> cachedVertices;
  unsigned int previousCachedVertices = 0;
  std::vector<unsigned int> indices;
  std::vector<Vertex> vertices;

  if (!modelFile.good())
    throw "Could not open model file";

  int l = 0;
  while (modelFile.good()) {
    l++;
    modelFile.getline(lineBuffer, bufSize);
    if (lineBuffer[0] == '#')
      continue;
    int space = 0;
    while (lineBuffer[space] != '\0' && lineBuffer[space] != ' ')
      space++;
    if (lineBuffer[space] == '\0')
      continue;
    std::stringstream ss;
    ss.str(lineBuffer + space + 1);

    float f1, f2, f3;
    int i1=0, i2=0, i3=0;
    if (strstr(lineBuffer, "v ") == lineBuffer) { // vertex
      ss >> f1 >> f2 >> f3;
      positions.push_back({ f1, f2, f3 });

    } else if (strstr(lineBuffer, "vt ") == lineBuffer) { // texture coordinate
      ss >> f1 >> f2;
      uvs.push_back({ f1, f2 });

    } else if (strstr(lineBuffer, "vn ") == lineBuffer) { // normal
      ss >> f1 >> f2 >> f3;
      normals.push_back({ f1, f2, f3 });

    } else if (strstr(lineBuffer, "f ") == lineBuffer) { // face
        for (size_t i = 0; i < 3; i++) {
            ss >> i1;                      // read vertex
            skipStreamText(ss, "/");
            if (ss.peek() != '/') ss >> i2; // read optional uv
            skipStreamText(ss, "/");
            ss >> i3;                      // read normal
            std::tuple<int, int, int> cacheKey{ i1, i2, i3 };
            auto inCacheIndex = std::find(cachedVertices.begin(), cachedVertices.end(), cacheKey);
            if (inCacheIndex == cachedVertices.end()) {
                vertices.emplace_back(Vertex{ positions[i1], uvs[i2], normals[i3], {1,0,0}, (float)currentTextureSlot });
                indices.push_back(previousCachedVertices + (unsigned int)cachedVertices.size());
                cachedVertices.push_back(cacheKey);
            } else {
                indices.push_back(previousCachedVertices + (unsigned int)(inCacheIndex - cachedVertices.begin()));
            }
        }

    } else if (strstr(lineBuffer, "mtllib ") == lineBuffer)  {
        ss >>  mtllib ;
        //std::cout << "Using material libraire : " + mtllib << std::endl;

    } else if (strstr(lineBuffer, "usemtl ") == lineBuffer)  {
        std::string material_name;
        ss >> material_name;
        materials_slots.push_back(material_name);
        currentTextureSlot++;

        previousCachedVertices = (unsigned int)vertices.size();
        cachedVertices.clear();

    } else { // unrecognized line

      continue;
    }
  }

  /* Tetxure loading */ 

  if (!mtllib.empty()) {

      std::string matlib_path = objPath.string().substr(0, objPath.string().find_last_of("/")+1) + mtllib;
      std::ifstream matfile{ matlib_path };
      char buf[bufSize];
      std::string cachedMaterialName;

      // This map stores a key Material and the texture file attached to it (in the mtl file)
      std::unordered_map<std::string, std::string> material_texturefilepath_map;

      /* Read the file and compute the hashmap accordingly */
      while (matfile.good()) {

          matfile.getline(buf, bufSize);
          if (buf[0] == '#')
              continue;
          int space = 0;
          while (buf[space] != '\0' && buf[space] != ' ')
              space++;
          if (buf[space] == '\0')
              continue;
          std::stringstream ss;
          ss.str(buf + space + 1);
          if (strstr(buf, "newmtl ") == buf) {
              
              ss >> cachedMaterialName;
          }
          else if (strstr(buf, "map_Kd ") == buf) {

              std::string textureFileName;
              ss >> textureFileName;

         
              std::string file_path = "res/textures/" + textureFileName;

              //std::cout << "Material : " + cachedMaterialName + " uses texture : " + textureFileName << std::endl;
              
              material_texturefilepath_map[cachedMaterialName] = file_path;


          }

      }


      // Loop through the materials found in the obj file, and produce ptr to textures for each material
      for (int i = 0; i < materials_slots.size(); i++) {

          const std::string& material_name = materials_slots[i];
            
          std::shared_ptr<Texture> computed;

          std::string texturePath;
          if (!material_texturefilepath_map.contains(material_name)) {

              std::cout << " === Warning : texture \" " << material_name << " \" has no texture in " + mtllib + " file !" << std::endl;
              computed = std::make_shared<Texture>("res/textures/no_texture.png");
              continue;
          } 
          struct stat buffer;
          texturePath = material_texturefilepath_map[material_name];;
          if ((stat(texturePath.c_str(), &buffer) != 0)) {
              std::cout << " === Warning : Texture \" " << texturePath << " \" was not found ! " << std::endl;
              computed = std::make_shared<Texture>("res/textures/no_texture.png");
          }
          else {
              computed = std::make_shared<Texture>(texturePath);
          }


          slotsTextures[i] = computed;
      }

  }

  return Mesh(vertices, indices, slotsTextures);
}

void clear()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void init()
{
  VertexBufferLayout emptyLayout;
  s_keepAliveResources = new KeepAliveResources;

  s_keepAliveResources->standardMeshShader = loadShaderFromFiles("res/shaders/standard.vs", "res/shaders/standard_color.fs"); // invalid shader
  //s_keepAliveResources->standardLightsShader = loadShaderFromFiles("res/shaders/standard.vs", "res/shaders/lights_pointlights.fs");
  s_keepAliveResources->standardLineShader = loadShaderFromFiles("res/shaders/standard_line.vs", "res/shaders/standard_color.fs");
  s_keepAliveResources->standardDepthPassShader = loadShaderFromFiles("res/shaders/depth_pass.vs", "res/shaders/depth_pass.fs");
  s_keepAliveResources->cubemapShader = loadShaderFromFiles("res/shaders/cubemap.vs", "res/shaders/cubemap.fs");
  
  s_keepAliveResources->lineIBO = IndexBufferObject({ 0, 1 });
  s_keepAliveResources->lineVAO.addBuffer(s_keepAliveResources->emptyVBO, emptyLayout, s_keepAliveResources->lineIBO);

  s_keepAliveResources->debugCubeMesh = createCubeMesh();
  s_keepAliveResources->debugCubeShader = loadShaderFromFiles("res/shaders/standard.vs", "res/shaders/standard_color.fs");
  s_keepAliveResources->debugNormalsShader = loadShaderFromFiles("res/shaders/standard.vs", "res/shaders/standard_color.fs");
  s_keepAliveResources->debugFlatScreenShader = loadShaderFromFiles("res/shaders/debugFlatScreen.vs", "res/shaders/debugFlatScreen.fs");

  { // cubemap VAO setup
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

    // expand the box for it to not go outside of orthographic cameras bounding boxes
    // (this requires all camera to have zFar be at least 25*sqrt(3) to see the full cuboid)
    for (float &v : vertices)
      v *= 50.f;

    unsigned int indices[] = {
      0, 1, 3, 3, 1, 2,
      1, 5, 2, 2, 5, 6,
      5, 4, 6, 6, 4, 7,
      4, 0, 7, 7, 0, 3,
      3, 2, 7, 7, 2, 6,
      4, 5, 0, 0, 5, 1
    };

    s_keepAliveResources->cubemapVBO = VertexBufferObject(vertices, sizeof(vertices));
    s_keepAliveResources->cubemapIBO = IndexBufferObject(indices, sizeof(indices) / sizeof(indices[0]));

    VertexBufferLayout layout;
    layout.push<float>(3);
    s_keepAliveResources->cubemapVAO.addBuffer(s_keepAliveResources->cubemapVBO, layout, s_keepAliveResources->cubemapIBO);
  }

  s_state.activeStandardShader = &s_keepAliveResources->standardMeshShader;

  int samplers[8] = { 0,1,2,3,4,5,6,7 };
  s_state.activeStandardShader->bind();
  s_state.activeStandardShader->setUniform1iv("u_Textures2D", 8, samplers);
  s_state.activeStandardShader->unbind();
  VertexArray::unbind();
}

void shutdown()
{
  delete s_keepAliveResources;
}

Shader &rebuildStandardMeshShader(const ShaderFactory &builder)
{
  if (s_keepAliveResources == nullptr)
    throw "Cannot fetch resources while the renderer is uninitialized";
  return s_keepAliveResources->standardMeshShader = builder.build();
}

Shader &getStandardMeshShader()
{
  if (s_keepAliveResources == nullptr)
    throw "Cannot fetch resources while the renderer is uninitialized";
  return s_keepAliveResources->standardMeshShader;
}

void beginColorPass()
{
  glEnable(GL_MULTISAMPLE);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  s_state.activeStandardShader = &s_keepAliveResources->standardMeshShader;
}

void beginDepthPass()
{
  glDisable(GL_MULTISAMPLE);
  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);  // do not draw color during a depth pass
  s_state.activeStandardShader = &s_keepAliveResources->standardDepthPassShader;
}

void renderMesh(const Camera &camera, const glm::vec3 &position, const glm::vec3 &size, const Mesh &mesh)
{
  s_debugData.meshCount++;
  s_debugData.vertexCount += mesh.getVertexCount();

  glm::mat4 M(1.f);
  M = glm::translate(M, position);
  M = glm::scale(M, size);
  s_state.activeStandardShader->bind();
  s_state.activeStandardShader->setUniform3f("u_cameraPos", camera.getPosition());
  s_state.activeStandardShader->setUniformMat4f("u_M", M);
  s_state.activeStandardShader->setUniformMat4f("u_VP", camera.getViewProjectionMatrix());
  mesh.draw();
  s_keepAliveResources->standardMeshShader.unbind();
}

void renderNormalsMesh(const Camera &camera, const glm::vec3 &position, const glm::vec3 &size, const NormalsMesh &normalsMesh, const glm::vec4 &color)
{
  s_debugData.meshCount++;
  s_debugData.debugLines += normalsMesh.getVertexCount()/2;

  glm::mat4 M(1.f);
  M = glm::translate(M, position);
  M = glm::scale(M, size);
  s_keepAliveResources->debugNormalsShader.bind();
  s_keepAliveResources->debugNormalsShader.setUniform4f("u_color", color);
  s_keepAliveResources->debugNormalsShader.setUniformMat4f("u_M", M);
  s_keepAliveResources->debugNormalsShader.setUniformMat4f("u_VP", camera.getViewProjectionMatrix());
  normalsMesh.draw();
}

void renderCubemap(const Camera &camera, const Cubemap &cubemap)
{
  s_keepAliveResources->cubemapVAO.bind();
  s_keepAliveResources->cubemapShader.bind();
  s_keepAliveResources->cubemapShader.setUniformMat4f("u_VP", camera.getViewProjectionMatrix());
  s_keepAliveResources->cubemapShader.setUniform3f("u_displacement", camera.getPosition());
  cubemap.bind();

  /*
  To draw the cubemap behind every other element the depth buffer is used in a peculiar way:
  When glClear is called, the depth buffer is cleared to 1. When elements are drawn the depth
  buffer is written to to <1. When the cubemap is drawn the DEPTH_FUNC is changed to only draw 
  fragments that have a depth value equal to that of the depth buffer, this way the cubemap
  shader can output 1 and only draw where other fragments have not been drawn.
  Note that opengl divides the screen position of the vertex shader by its w component, therefore
  the shader should not set the screen position to 1 but rather to gl_Position.w
  */

  glDepthFunc(GL_LEQUAL);
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
  glDepthFunc(GL_LESS);

  VertexArray::unbind();
  s_keepAliveResources->cubemapShader.unbind();
}

void renderDebugLine(const Camera &camera, const glm::vec3 &from, const glm::vec3 &to, const glm::vec4 &color)
{
  s_debugData.debugLines++;
  s_debugData.vertexCount += 2;
  s_keepAliveResources->lineVAO.bind();
  s_keepAliveResources->standardLineShader.bind();
  s_keepAliveResources->standardLineShader.setUniform3f("u_from", from);
  s_keepAliveResources->standardLineShader.setUniform3f("u_to", to);
  s_keepAliveResources->standardLineShader.setUniform4f("u_color", color);
  s_keepAliveResources->standardLineShader.setUniformMat4f("u_VP", camera.getViewProjectionMatrix());
  glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, nullptr);
}

void renderDebugCube(const Camera &camera, const glm::vec3 &position, const glm::vec3 &size, const glm::vec4 &color)
{
  glm::mat4 M(1.f);
  M = glm::translate(M, position);
  M = glm::scale(M, size);
  s_keepAliveResources->debugCubeShader.bind();
  s_keepAliveResources->debugCubeShader.setUniform4f("u_color", color);
  s_keepAliveResources->debugCubeShader.setUniformMat4f("u_M", M);
  s_keepAliveResources->debugCubeShader.setUniformMat4f("u_VP", camera.getViewProjectionMatrix());
  s_keepAliveResources->debugCubeMesh.draw();
}

void renderDebugAxis(const Camera &camera)
{
  renderDebugLine(camera, { 0, 0, 0 }, { 10, 0, 0 }, { 1.f, 0.f, 0.f, 1.f }); // x red
  renderDebugLine(camera, { 0, 0, 0 }, { 0, 10, 0 }, { 0.f, 1.f, 0.f, 1.f }); // y green
  renderDebugLine(camera, { 0, 0, 0 }, { 0, 0, 10 }, { 0.f, 0.f, 1.f, 1.f }); // z blue
}


void renderAABBDebugOutline(const Camera &camera, const AABB &aabb, const glm::vec4 &color)
{
  glm::vec3 o = aabb.getOrigin();
  glm::vec3 x = { aabb.getSize().x, 0, 0 };
  glm::vec3 y = { 0, aabb.getSize().y, 0 };
  glm::vec3 z = { 0, 0, aabb.getSize().z };
  renderDebugLine(camera, o, o + x, color);
  renderDebugLine(camera, o + y, o + x + y, color);
  renderDebugLine(camera, o + z, o + x + z, color);
  renderDebugLine(camera, o + y + z, o + x + y + z, color);
  renderDebugLine(camera, o, o + y, color);
  renderDebugLine(camera, o + x, o + y + x, color);
  renderDebugLine(camera, o + z, o + y + z, color);
  renderDebugLine(camera, o + x + z, o + y + x + z, color);
  renderDebugLine(camera, o, o + z, color);
  renderDebugLine(camera, o + x, o + z + x, color);
  renderDebugLine(camera, o + y, o + z + y, color);
  renderDebugLine(camera, o + x + y, o + z + x + y, color);
}

static void renderDebugOrthographicCameraOutline(const Camera &viewCamera, const Camera &outlinedCamera)
{
  const OrthographicProjection &proj = outlinedCamera.getProjection<OrthographicProjection>();
  const glm::vec3 pos = outlinedCamera.getPosition();
  glm::vec3 I = outlinedCamera.getRight();
  glm::vec3 J = outlinedCamera.getUp();
  glm::vec3 F = outlinedCamera.getForward();
  float zNear = proj.zNear;
  float zFar = proj.zFar;
  glm::vec3 p1 = pos + I * proj.right + J * proj.top;
  glm::vec3 p2 = pos + I * proj.right - J * proj.top;
  glm::vec3 p3 = pos - I * proj.right - J * proj.top;
  glm::vec3 p4 = pos - I * proj.right + J * proj.top;

  renderDebugLine(viewCamera, pos, pos + F * zFar, { 1.f, 0.f, .0f, 1.f }); // dir
  renderDebugLine(viewCamera, pos, pos + Camera::UP, { 1.f, 1.f, .3f, 1.f }); // world up
  renderDebugLine(viewCamera, pos, pos + I, { .5f, 1.f, .5f, 1.f }); // right
  renderDebugLine(viewCamera, pos, pos + J, { 1.f, .5f, .5f, 1.f }); // up
  renderDebugLine(viewCamera, p1, p1 + F * zFar, { .5f, .5f, .5f, 1.f });
  renderDebugLine(viewCamera, p2, p2 + F * zFar, { .5f, .5f, .5f, 1.f });
  renderDebugLine(viewCamera, p3, p3 + F * zFar, { .5f, .5f, .5f, 1.f });
  renderDebugLine(viewCamera, p4, p4 + F * zFar, { .5f, .5f, .5f, 1.f });
  for (float z = zNear; z < zFar; z += 5) {
    renderDebugLine(viewCamera, p1 + z * F, p2 + z * F, { .5f, .5f, .5f, 1.f });
    renderDebugLine(viewCamera, p2 + z * F, p3 + z * F, { .5f, .5f, .5f, 1.f });
    renderDebugLine(viewCamera, p3 + z * F, p4 + z * F, { .5f, .5f, .5f, 1.f });
    renderDebugLine(viewCamera, p4 + z * F, p1 + z * F, { .5f, .5f, .5f, 1.f });
  }
  renderDebugCube(viewCamera, pos, { .1f, .1f, .1f });
}

// this is off by a very small margin, no idea why, propably not worth it to investigate
static void renderDebugPerspectiveCameraOutline(const Camera &viewCamera, const Camera &outlinedCamera)
{
  const PerspectiveProjection &proj = outlinedCamera.getProjection<PerspectiveProjection>();
  glm::vec3 pos = outlinedCamera.getPosition();
  glm::vec3 I = outlinedCamera.getRight();
  glm::vec3 J = outlinedCamera.getUp();
  glm::vec3 F = outlinedCamera.getForward();
  float dh = sin(proj.fovy * .5f);
  float dw = dh * proj.aspect;
  glm::vec3 U1 = F + dh * J + dw * I;
  glm::vec3 U2 = F - dh * J + dw * I;
  glm::vec3 U3 = F - dh * J - dw * I;
  glm::vec3 U4 = F + dh * J - dw * I;
  float zNear = proj.zNear;
  float zFar = proj.zFar;

  renderDebugLine(viewCamera, pos, pos + F * zFar, { 1.f, 0.f, .0f, 1.f }); // dir
  renderDebugLine(viewCamera, pos, pos + Camera::UP, { 1.f, 1.f, .3f, 1.f }); // world up
  renderDebugLine(viewCamera, pos, pos + I, { .5f, 1.f, .5f, 1.f }); // right
  renderDebugLine(viewCamera, pos, pos + J, { 1.f, .5f, .5f, 1.f }); // up
  renderDebugLine(viewCamera, pos, pos + U1 * zFar, { .5f, .5f, .5f, 1.f });
  renderDebugLine(viewCamera, pos, pos + U2 * zFar, { .5f, .5f, .5f, 1.f });
  renderDebugLine(viewCamera, pos, pos + U3 * zFar, { .5f, .5f, .5f, 1.f });
  renderDebugLine(viewCamera, pos, pos + U4 * zFar, { .5f, .5f, .5f, 1.f });
  renderDebugLine(viewCamera, pos + U1 * zFar, pos + U2 * zFar, { .5f, .5f, .5f, 1.f });
  renderDebugLine(viewCamera, pos + U2 * zFar, pos + U3 * zFar, { .5f, .5f, .5f, 1.f });
  renderDebugLine(viewCamera, pos + U3 * zFar, pos + U4 * zFar, { .5f, .5f, .5f, 1.f });
  renderDebugLine(viewCamera, pos + U4 * zFar, pos + U1 * zFar, { .5f, .5f, .5f, 1.f });
  for (float z = zNear; z < zFar; z += 5) {
    renderDebugLine(viewCamera, pos + U1 * z, pos + U2 * z, { .5f, .5f, .5f, 1.f });
    renderDebugLine(viewCamera, pos + U2 * z, pos + U3 * z, { .5f, .5f, .5f, 1.f });
    renderDebugLine(viewCamera, pos + U3 * z, pos + U4 * z, { .5f, .5f, .5f, 1.f });
    renderDebugLine(viewCamera, pos + U4 * z, pos + U1 * z, { .5f, .5f, .5f, 1.f });
  }
  renderDebugCube(viewCamera, pos, { .1f, .1f, .1f });
}

void renderDebugCameraOutline(const Camera &viewCamera, const Camera &outlinedCamera)
{
  switch (outlinedCamera.getProjectionType()) {
  case CameraProjection::ORTHOGRAPHIC: renderDebugOrthographicCameraOutline(viewCamera, outlinedCamera);       break;
  case CameraProjection::PERSPECTIVE:  renderDebugPerspectiveCameraOutline(viewCamera, outlinedCamera); break;
  default:                             throw "Unreachable";                             break;
  }
}


void renderDebugGUIQuadWithTexture(const Texture& texture, glm::vec2 positionOnScreen, glm::vec2 size)
{
    // TODO store a plane mesh in a static variable and rotate,translate&scale at each call
    std::vector<Vertex> vertices{
        // position                                                          uv            normal (up)    color
        { { positionOnScreen.x,          positionOnScreen.y,          0.f }, { 1.f, 1.f }, { 0, 1.f, 0 }, {1.0f, 1.0f, 0.0f}, },
        { { positionOnScreen.x,          positionOnScreen.y + size.y, 0.f }, { 1.f, 0.f }, { 0, 1.f, 0 }, {1.0f, 1.0f, 0.0f}, },
        { { positionOnScreen.x + size.x, positionOnScreen.y + size.y, 0.f }, { 0.f, 0.f }, { 0, 1.f, 0 }, {1.0f, 1.0f, 0.0f}, },
        { { positionOnScreen.x + size.x, positionOnScreen.y,          0.f }, { 0.f, 1.f }, { 0, 1.f, 0 }, {1.0f, 1.0f, 0.0f}, },
    };

    std::vector<unsigned int> indices{
      3, 2, 0, 1,0,2
    };

    Mesh gui{ vertices, indices };

    s_keepAliveResources->debugFlatScreenShader.bind();
    texture.bind(0);
    s_keepAliveResources->debugFlatScreenShader.setUniform1i("u_texture", 0);
    gui.draw();
}

void setUniformPointLights(const std::vector<Light>& pointLights)
{
    s_keepAliveResources->standardLightsShader.bind();
    s_keepAliveResources->standardLightsShader.setUniform1i("u_numberOfLights", (int)pointLights.size());

    for (int i = 0; i < pointLights.size(); i++) {
        const Light& light = pointLights.at(i);

        std::stringstream ss{ std::string() };
        ss << "u_lights[";
        ss << i;
        ss << "].";
        std::string lightInShader = ss.str();
        s_keepAliveResources->standardLightsShader.setUniform1i(lightInShader + "on", light.isOn());

        s_keepAliveResources->standardLightsShader.setUniform3f(lightInShader + "position", light.getPosition());

        s_keepAliveResources->standardLightsShader.setUniform1f(lightInShader + "constant", light.getCoefs().constant);
        s_keepAliveResources->standardLightsShader.setUniform1f(lightInShader + "linear", light.getCoefs().linear);
        s_keepAliveResources->standardLightsShader.setUniform1f(lightInShader + "quadratic", light.getCoefs().quadratic);

        s_keepAliveResources->standardLightsShader.setUniform3f(lightInShader + "ambient", light.getParams().ambiant);
        s_keepAliveResources->standardLightsShader.setUniform3f(lightInShader + "diffuse", light.getParams().diffuse);
        s_keepAliveResources->standardLightsShader.setUniform3f(lightInShader + "specular", light.getParams().specular);
    }
    s_keepAliveResources->standardLightsShader.unbind();
}

//=========================================================================================================================//
//=========================================================================================================================//
//=========================================================================================================================//
//=========================================================================================================================//

void clearDebugData() {
  s_debugData.meshCount = 0;
  s_debugData.vertexCount = 0;
  s_debugData.debugLines = 0;
}

const DebugData& getRendererDebugData() {
  return s_debugData;
}

}