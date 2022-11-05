#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../Scene.h"
#include "../../abstraction/Renderer.h"
#include "../../abstraction/Cubemap.h"
#include "../../abstraction/UnifiedRenderer.h"
#include "../../World/TerrainGeneration/HeightMap.h"
#include "../../World/TerrainGeneration/MeshGenerator.h"
#include "../../World/Player.h"
#include "../../Utils/Mathf.h"
#include "../../Utils/MathIterators.h"

namespace Renderer {

class InstancedMesh {
public:
  struct ModelVertex {
    glm::vec3 position; // model position
  };
  struct InstanceData {
    glm::vec4 position;
    float colorPalette;
    char _padding[3*sizeof(float)];
  };

private:
public:
  VertexBufferObject m_modelVBO, m_instanceVBO;
  IndexBufferObject m_ibo;
  VertexArray m_vao;
  unsigned int m_instanceCount;

public:
  InstancedMesh()
    : m_modelVBO(), m_instanceVBO(), m_ibo(), m_vao(), m_instanceCount(0)
  {
  }

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
    instanceLayout.push<float>(4); // position&height
    instanceLayout.push<float>(1); // color palette
    instanceLayout.push<float>(3); // padding
    m_vao.addBuffer(m_modelVBO, modelLayout, m_ibo);
    m_vao.addInstanceBuffer(m_instanceVBO, instanceLayout, modelLayout);
    m_vao.unbind();
  }

  InstancedMesh(const std::vector<ModelVertex> &vertices, const std::vector<unsigned int> &indices, size_t instanceBufferSize)
    : m_modelVBO(vertices.data(), vertices.size() * sizeof(ModelVertex)),
    m_instanceVBO(instanceBufferSize),
    m_ibo(indices.data(), indices.size()),
    m_instanceCount(0),
    m_vao()
  {
    VertexBufferLayout modelLayout;
    VertexBufferLayout instanceLayout;
    modelLayout.push<float>(3); // position
    instanceLayout.push<float>(4); // position&height
    instanceLayout.push<float>(1); // color palette
    instanceLayout.push<float>(3); // padding
    m_vao.addBuffer(m_modelVBO, modelLayout, m_ibo);
    m_vao.addInstanceBuffer(m_instanceVBO, instanceLayout, modelLayout);
    m_vao.unbind();
  }

  InstancedMesh(InstancedMesh &&moved) noexcept
  {
    m_modelVBO = std::move(moved.m_modelVBO);
    m_instanceVBO = std::move(moved.m_instanceVBO);
    m_ibo = std::move(moved.m_ibo);
    m_vao = std::move(moved.m_vao);
    m_instanceCount = moved.m_instanceCount;
  }

  InstancedMesh &operator=(InstancedMesh &&moved) noexcept
  {
    this->~InstancedMesh();
    new (this)InstancedMesh(std::move(moved));
    return *this;
  }

  InstancedMesh(const InstancedMesh &) = delete;
  InstancedMesh &operator=(InstancedMesh &moved) = delete;

  void draw() const
  {
    m_vao.bind();
    glDrawElementsInstanced(GL_TRIANGLES, m_ibo.getCount(), GL_UNSIGNED_INT, nullptr, m_instanceCount);
    m_vao.unbind();
  }
};

}

constexpr int SSBO_ALIGNMENT_SIZE = 4; // 32bits per "basic machine unit" (uint)

static int getSSBOBaseAligment(GLuint type)
{
  switch (type) {
  case GL_INT: return 1;
  case GL_BOOL: return 1;
  default: throw std::exception("Unreachable");
  }
}

struct IndirectDrawCommand {
  GLuint count;
  GLuint instanceCount;
  GLuint firstIndex;
  GLint  baseVertex;
  GLuint baseInstance;
};

class TestInstancedScene : public Scene {
private:
  static constexpr unsigned int CHUNK_COUNT_X = 4, CHUNK_COUNT_Y = 4, CHUNK_SIZE = 25;
  static constexpr float GRASS_DENSITY = 2.5f; // grass blade per square meter
  static constexpr int BLADES_PER_CHUNK = Mathf::ceilToPowerOfTwo((CHUNK_SIZE * CHUNK_SIZE) * (GRASS_DENSITY * GRASS_DENSITY));
  static constexpr int TOTAL_BLADE_COUNT = CHUNK_COUNT_X * CHUNK_COUNT_Y * BLADES_PER_CHUNK;
  Renderer::Cubemap   m_skybox;
  Player              m_player, m_roguePlayer;
  bool                m_useRoguePlayer;
  Renderer::Texture   m_texture1;
  Renderer::Shader    m_grassShader;
  Renderer::InstancedMesh m_grassMesh;
  TerrainMeshGenerator::Terrain m_terrain;

  unsigned int m_voteComputeShader;
  unsigned int m_scanComputeShader;
  unsigned int m_compactComputeShader;
  unsigned int cullingBuffer, runningSumBuffer, runningSumTempBuffer, initialInstancesBuffer, indirectDrawBuffer;

public:
  TestInstancedScene()
    : m_skybox{
      "res/skybox_dbg/skybox_front.bmp", "res/skybox_dbg/skybox_back.bmp",
      "res/skybox_dbg/skybox_left.bmp",  "res/skybox_dbg/skybox_right.bmp",
      "res/skybox_dbg/skybox_top.bmp",   "res/skybox_dbg/skybox_bottom.bmp" },
      m_texture1("res/textures/rock.jpg"),
      m_player(), m_roguePlayer(), m_useRoguePlayer(false)
  {
    m_player.setPostion({ 125, 10, 0 });
    m_player.setRotation(3.14f*3/4.f, 0);
    m_player.updateCamera();

    TerrainMeshGenerator::TerrainData terrainData{};
    terrainData.scale = 100.f;
    terrainData.octaves = 3;
    terrainData.seed = 0;
    m_terrain = TerrainMeshGenerator::generateTerrain(
      terrainData,
      CHUNK_COUNT_X, CHUNK_COUNT_Y,
      CHUNK_SIZE
    );
    Renderer::getStandardMeshShader().bind();
    Renderer::getStandardMeshShader().setUniform1i("u_RenderChunks", 1);

    m_grassMesh = generateGrassMesh();

    m_grassShader = Renderer::Shader(R"glsl(
#version 430 core

layout(std430, binding = 1) buffer o_culled {
  bool culled_instances[];
};

layout(location = 0) in vec3 im_position;
layout(location = 1) in vec4 ii_position;
layout(location = 2) in float ii_colorPalette;

out float o_fragmentHeight;
out float o_colorPalette;

uniform mat4 u_VP;
uniform mat2 u_R;

void main()
{
  o_fragmentHeight = im_position.y;
  //o_colorPalette = ii_colorPalette;
  o_colorPalette = culled_instances[gl_InstanceID] ? 1 : 0;
  vec4 vertex = vec4(im_position, 1);
  vertex.xz = u_R * vertex.xz;
  vertex.y *= ii_position.w;
  vertex.xyz += ii_position.xyz;
  vertex = u_VP * vertex;
  gl_Position = vertex;
}

)glsl", R"glsl(
#version 330 core

out vec4 color;

in float o_fragmentHeight;
in float o_colorPalette;

const vec3 bladeColor1 = vec3(0, .14, 0);
const vec3 bladeColor2 = vec3(.05, .8, .41);

void main()
{
  color = vec4(mix(bladeColor1, bladeColor2, o_fragmentHeight), 1);
}
)glsl");

    m_voteComputeShader = createComputeShader(voteComputeSource);
    m_scanComputeShader = createComputeShader(scanTrueComputeSource);
    m_compactComputeShader = createComputeShader(compactComputeSource);
    
    glCreateBuffers(1, &cullingBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, cullingBuffer);
    glBufferData(GL_ARRAY_BUFFER, getSSBOBaseAligment(GL_BOOL) * SSBO_ALIGNMENT_SIZE * TOTAL_BLADE_COUNT, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glCreateBuffers(1, &runningSumBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, runningSumBuffer);
    glBufferData(GL_ARRAY_BUFFER, getSSBOBaseAligment(GL_INT) * SSBO_ALIGNMENT_SIZE * TOTAL_BLADE_COUNT, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glCreateBuffers(1, &runningSumTempBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, runningSumBuffer);
    glBufferData(GL_ARRAY_BUFFER, getSSBOBaseAligment(GL_INT) * SSBO_ALIGNMENT_SIZE * TOTAL_BLADE_COUNT * 2, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    std::vector<Renderer::InstancedMesh::InstanceData> instances = generateBladesInstances();
    glCreateBuffers(1, &initialInstancesBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, initialInstancesBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Renderer::InstancedMesh::InstanceData) * instances.size(), instances.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    int *buf = new int[N], *targetBuf = new int[N];
    for (int i = 0; i < N; i++)
      buf[i] = i;
    glBindBuffer(GL_ARRAY_BUFFER, cullingBuffer);
    glBufferData(GL_ARRAY_BUFFER, N * sizeof(int), buf, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, runningSumBuffer);
    glBufferData(GL_ARRAY_BUFFER, N * sizeof(int), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, runningSumTempBuffer);
    glBufferData(GL_ARRAY_BUFFER, N * sizeof(int)*2, nullptr, GL_DYNAMIC_DRAW);
    glUseProgram(m_scanComputeShader);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, cullingBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, runningSumBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, runningSumTempBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, indirectDrawBuffer);
    glUniform1ui(glGetUniformLocation(m_scanComputeShader, "u_bladesCount"), N);
    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    glBindBuffer(GL_ARRAY_BUFFER, runningSumBuffer);
    glGetBufferSubData(GL_ARRAY_BUFFER, 0, N * sizeof(int), targetBuf);
    int s = 0;
    for (int i = 0; i < N; i++) {
      if(targetBuf[i] != s)
        std::cout << i << ": got " << targetBuf[i] << " expected " << s << std::endl;
      s += buf[i];
    }
    std::cout << ">" << std::endl;
    //for (int i = 0; i < sizeof(buf) / sizeof(buf[0]); i++)
    //  std::cout << i << ": " << buf[i] << std::endl;

    IndirectDrawCommand drawCommand{};
    drawCommand.count = m_grassMesh.m_ibo.getCount();
    drawCommand.firstIndex = 0;
    drawCommand.instanceCount = -1; // to be filled by compute shaders
    drawCommand.baseInstance = 0;
    drawCommand.baseVertex = 0;
    glCreateBuffers(1, &indirectDrawBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, indirectDrawBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(IndirectDrawCommand), &drawCommand, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //glShaderStorageBlockBinding(m_voteComputeShader, glGetProgramResourceIndex(m_voteComputeShader, GL_SHADER_STORAGE_BLOCK, "i_instances"), 0);
    //glShaderStorageBlockBinding(m_voteComputeShader, glGetProgramResourceIndex(m_voteComputeShader, GL_SHADER_STORAGE_BLOCK, "o_culled"), 1);
  }

  static unsigned int createComputeShader(const char *source)
  {
    unsigned int computeShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(computeShader, 1, &source, NULL);
    glCompileShader(computeShader);
    unsigned int program = glCreateProgram();
    glAttachShader(program, computeShader);
    glLinkProgram(program);
    glDeleteShader(computeShader);
    return program;
  }

  Renderer::InstancedMesh generateGrassMesh()
  {
    std::vector<Renderer::InstancedMesh::ModelVertex> bladeVertices;
    std::vector<unsigned int> bladeIndices;
    float l = .1f;
    float s = .07f;
    float k = .03f;
    float h = .5f;
    float m = .85f;
    float t = 1.f;
    bladeVertices.push_back({ {-l,0,0} });
    bladeVertices.push_back({ {l,0,0} });
    bladeVertices.push_back({ {-s,h,0} });
    bladeVertices.push_back({ {s,h,0} });
    bladeVertices.push_back({ {-k,m,0} });
    bladeVertices.push_back({ {k,m,0} });
    bladeVertices.push_back({ {0,t,0} });
    bladeIndices.push_back(0); bladeIndices.push_back(2); bladeIndices.push_back(1);
    bladeIndices.push_back(1); bladeIndices.push_back(2); bladeIndices.push_back(3);
    bladeIndices.push_back(2); bladeIndices.push_back(4); bladeIndices.push_back(3);
    bladeIndices.push_back(3); bladeIndices.push_back(4); bladeIndices.push_back(5);
    bladeIndices.push_back(5); bladeIndices.push_back(4); bladeIndices.push_back(6);
    return Renderer::InstancedMesh(bladeVertices, bladeIndices, TOTAL_BLADE_COUNT * sizeof(Renderer::InstancedMesh::InstanceData));
  }

  std::vector<Renderer::InstancedMesh::InstanceData> generateBladesInstances()
  {
    std::vector<Renderer::InstancedMesh::InstanceData> instances;
    int i = 0;
    //float s = glm::sqrt(BLADES_PER_CHUNK);
    for (int cx = 0; cx < CHUNK_COUNT_X; cx++) {
      for (int cy = 0; cy < CHUNK_COUNT_Y; cy++) {
        // generates blades for one chunk
        for (int b = 0; b < BLADES_PER_CHUNK; b++) {
          float r = b + cx*.252f + cy*.62f;
          float bladeX = (cx + Mathf::rand(r)) * CHUNK_SIZE;
          float bladeZ = (cy + Mathf::rand(-r+2.4f)) * CHUNK_SIZE;
          //float bladeX = (cx + b/(int)s/(float)s) * CHUNK_SIZE;
          //float bladeZ = (cy + b%(int)s/(float)s) * CHUNK_SIZE;
          float bladeY = m_terrain.getHeight(bladeX, bladeZ);
          float bladeHeight = 1.f + i/10000.f;
          Renderer::InstancedMesh::InstanceData blade{};
          blade.position = { bladeX, bladeY, bladeZ, bladeHeight };
          blade.colorPalette = 0.f;
          instances.push_back(blade);
          i++;
        }
      }
    }
    std::cout << instances.size() << " grass blades" << std::endl;
    return instances;
  }

  void step(float delta) override
  {
    Player &activePlayer = m_useRoguePlayer ? m_roguePlayer : m_player;
    activePlayer.step(delta);
  }

  void onRender() override
  {
    Renderer::Renderer::clear();
    Renderer::CubemapRenderer::drawCubemap(m_skybox, m_player.getCamera());

    const Renderer::Camera &renderCamera = (m_useRoguePlayer ? m_roguePlayer : m_player).getCamera();
    m_texture1.bind(0);
    m_texture1.bind(1);
    for (const auto &[position, chunk] : m_terrain.getChunks()) {
      Renderer::renderMesh(glm::vec3{ 0 }, glm::vec3{ 1 }, chunk.getMesh(), renderCamera);

      if (DebugWindow::renderAABB())
        renderAABBDebugOutline(renderCamera, chunk.getMesh().getBoundingBox());
    }

    if (m_useRoguePlayer) {
      Renderer::renderDebugCameraOutline(renderCamera, m_player.getCamera());
    }



    const glm::mat4 &VP = m_player.getCamera().getViewProjectionMatrix();
    static_assert(Mathf::isPowerOfTwo(TOTAL_BLADE_COUNT));
    constexpr int groupCount = (TOTAL_BLADE_COUNT+1023/*ceil*/) / 1024;
    static_assert(groupCount < 1024); // do not overflow compute shader capacity

    glUseProgram(m_voteComputeShader);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, initialInstancesBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, cullingBuffer);
    glUniformMatrix4fv(glGetUniformLocation(m_voteComputeShader, "u_VP"), 1, GL_FALSE, glm::value_ptr(VP));
    glUniform1ui(glGetUniformLocation(m_voteComputeShader, "u_bladesCount"), TOTAL_BLADE_COUNT);
    glDispatchCompute(groupCount, 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    glUseProgram(m_scanComputeShader);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, cullingBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, runningSumBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, indirectDrawBuffer);
    glUniform1ui(glGetUniformLocation(m_scanComputeShader, "u_bladesCount"), TOTAL_BLADE_COUNT);
    glDispatchCompute(groupCount/2, 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    glUseProgram(m_compactComputeShader);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, initialInstancesBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, cullingBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, runningSumBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_grassMesh.m_instanceVBO.getId());
    glUniform1ui(glGetUniformLocation(m_compactComputeShader, "u_bladesCount"), TOTAL_BLADE_COUNT);
    glDispatchCompute(groupCount, 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    //constexpr size_t N = 50;
    //float buffer[N];
    //Renderer::InstancedMesh::InstanceData bladesBuffer[N];
    //glBindBuffer(GL_ARRAY_BUFFER, cullingBuffer);
    //glGetBufferSubData(GL_ARRAY_BUFFER, 0, getSSBOBaseAligment(GL_BOOL) * SSBO_ALIGNMENT_SIZE * N, buffer);
    //glBindBuffer(GL_ARRAY_BUFFER, m_grassMesh.m_instanceVBO.getId());
    //glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Renderer::InstancedMesh::InstanceData) * N, bladesBuffer);
    //for (int i = 0; i < N; i++) {
    //  std::cout << i << ": " << ((float *)bladesBuffer)[i] << " " << buffer[i] << std::endl;
    //}
      //std::cout << i << ": " << bladesBuffer[i].position.x << " " << buffer[i] << std::endl;

    float theta = 3.14f - renderCamera.getYaw();
    float c = cos(theta), s = sin(theta);
    glm::mat2 facingCameraRotationMatrix = glm::mat2(c, s, -s, c);

    m_grassShader.bind();
    m_grassShader.setUniformMat4f("u_VP", renderCamera.getViewProjectionMatrix());
    m_grassShader.setUniformMat2f("u_R", facingCameraRotationMatrix);
    
    //IndirectDrawCommand cmd;
    //glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectDrawBuffer);
    ////glGetBufferSubData(GL_DRAW_INDIRECT_BUFFER, 0, sizeof(IndirectDrawCommand), &cmd);
    //cmd.instanceCount = 1000;

    m_grassMesh.m_vao.bind();
    //glDrawElementsInstanced(GL_TRIANGLES, m_grassMesh.m_ibo.getCount(), GL_UNSIGNED_INT, nullptr, cmd.instanceCount);
    //((Renderer::DebugData&) Renderer::getRendererDebugData()).meshCount = cmd.instanceCount;
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectDrawBuffer);
    glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr);
    m_grassMesh.m_vao.unbind();
    //m_grassMesh.draw();
  }

  void onImGuiRender() override
  {
    if (ImGui::Begin(">")) {
      ImGui::Checkbox("rogue player", &m_useRoguePlayer);
      glm::vec3 pp = m_player.getPosition();
      glm::vec2 pr = m_player.getRotation();
      ImGui::DragFloat3("player position", glm::value_ptr(pp), .1f);
      ImGui::DragFloat2("player rotation", glm::value_ptr(pr), .1f);
      m_player.setPostion(pp);
      m_player.setRotation(pr.x, pr.y);
      m_player.updateCamera();
    }
    ImGui::End();
  }
};