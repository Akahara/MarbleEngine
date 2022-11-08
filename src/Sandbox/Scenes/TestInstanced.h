#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/component_wise.hpp>

#include "../Scene.h"
#include "../../abstraction/Renderer.h"
#include "../../abstraction/Cubemap.h"
#include "../../abstraction/UnifiedRenderer.h"
#include "../../World/TerrainGeneration/HeightMap.h"
#include "../../World/TerrainGeneration/MeshGenerator.h"
#include "../../World/Player.h"
#include "../../Utils/Mathf.h"
#include "../../Utils/MathIterators.h"
#include "../../Utils/StringUtils.h"

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
    m_instanceCount((unsigned int)instances.size()),
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
    glDrawElementsInstanced(GL_TRIANGLES, (unsigned int)m_ibo.getCount(), GL_UNSIGNED_INT, nullptr, m_instanceCount);
    m_vao.unbind();
  }
};

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
  Renderer::Cubemap   m_skybox;
  Player              m_player, m_roguePlayer;
  bool                m_useRoguePlayer;
  Renderer::Texture   m_texture1;
  Renderer::Shader    m_grassShader;
  Renderer::InstancedMesh m_grassMesh;
  TerrainMeshGenerator::Terrain m_terrain;

  unsigned int m_voteComputeShader;
  unsigned int m_scan1ComputeShader;
  unsigned int m_scan2ComputeShader;
  unsigned int m_scan3ComputeShader;
  unsigned int m_compactComputeShader;
  unsigned int m_instancesBuffer, m_bigBuffer;

  static constexpr unsigned int GROUP_COUNT = 256, GROUP_WORK = 1024;
  static constexpr int TOTAL_BLADE_COUNT = GROUP_COUNT * GROUP_WORK;
  static constexpr int BLADES_PER_CHUNK = TOTAL_BLADE_COUNT / CHUNK_COUNT_X / CHUNK_COUNT_Y;
  static_assert(BLADES_PER_CHUNK *CHUNK_COUNT_X * CHUNK_COUNT_Y == TOTAL_BLADE_COUNT);

  /*
   * This structure is never actually instantiated on the cpu be will be on the GPU, and the layout will remain the same.
   * It contains all the data space necessary for the vote-scan&compact algorithms.
   */
  struct BigBuffer {
    IndirectDrawCommand drawCommand; // must be the first member because BigBuffer will be used as a GL_DRAW_INDIRECT_BUFFER
    int voteBuffer[TOTAL_BLADE_COUNT];
    int scanBuffer[TOTAL_BLADE_COUNT];
    int scanTempBuffer[TOTAL_BLADE_COUNT];
    int totalsBuffer[GROUP_COUNT];
    int totalsTempBuffer[GROUP_COUNT];
  };

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

    m_grassShader = Renderer::loadShaderFromFiles("res/shaders/grass/grass.vs", "res/shaders/grass/grass.fs");

    m_voteComputeShader = createComputeShader("res/shaders/grass/vote.comp");
    m_scan1ComputeShader = createComputeShader("res/shaders/grass/scan_blocks.comp");
    m_scan2ComputeShader = createComputeShader("res/shaders/grass/scan_groups.comp");
    m_scan3ComputeShader = createComputeShader("res/shaders/grass/scan_accumulate.comp");
    m_compactComputeShader = createComputeShader("res/shaders/grass/compact.comp");

    std::vector<Renderer::InstancedMesh::InstanceData> instances = generateBladesInstances();
    IndirectDrawCommand filledDrawCommand{};
    filledDrawCommand.count = (GLuint)m_grassMesh.m_ibo.getCount();
    filledDrawCommand.firstIndex = 0;
    filledDrawCommand.instanceCount = -1; // to be filled by compute shaders
    filledDrawCommand.baseInstance = 0;
    filledDrawCommand.baseVertex = 0;

    glCreateBuffers(1, &m_bigBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_bigBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(BigBuffer), nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, offsetof(BigBuffer, drawCommand), sizeof(BigBuffer::drawCommand), &filledDrawCommand); // fill only the drawCommand part of the big buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glCreateBuffers(1, &m_instancesBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_instancesBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Renderer::InstancedMesh::InstanceData) * instances.size(), instances.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  static unsigned int createComputeShader(const char *sourcePath)
  {
    std::ifstream vertexFile{ sourcePath };
    std::stringstream buffer;
    buffer << vertexFile.rdbuf();
    std::string vertexCode = buffer.str();
    Utils::replaceAll(vertexCode, "{WORK_SIZE}", std::to_string(GROUP_WORK));
    Utils::replaceAll(vertexCode, "{GROUP_COUNT}", std::to_string(GROUP_COUNT));

    const char *vertexCodeCstr = vertexCode.c_str();
    unsigned int computeShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(computeShader, 1, &vertexCodeCstr, NULL);
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
          float bladeHeight = 1.f + Mathf::fract(r*634.532f)*.5f;
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


    // I/ vote
    glm::mat4 V = m_player.getCamera().getViewMatrix();
    V = glm::translate(V, m_player.getCamera().getForward() * 1.f); // move the clip camera back a bit to be sure blades that are very close to the actual camera do not get clipped
    glm::mat4 VP = m_player.getCamera().getProjectionMatrix() * V;
    glUseProgram(m_voteComputeShader);
    glUniformMatrix4fv(glGetUniformLocation(m_voteComputeShader, "u_VP"), 1, GL_FALSE, glm::value_ptr(VP));
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_instancesBuffer);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, m_bigBuffer, offsetof(BigBuffer, voteBuffer), sizeof(BigBuffer::voteBuffer));
    glDispatchCompute(GROUP_COUNT, 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    // II/ scan
    glUseProgram(m_scan1ComputeShader);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, m_bigBuffer, offsetof(BigBuffer, voteBuffer), sizeof(BigBuffer::voteBuffer));
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, m_bigBuffer, offsetof(BigBuffer, scanBuffer), sizeof(BigBuffer::scanBuffer) + sizeof(BigBuffer::scanTempBuffer));
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 2, m_bigBuffer, offsetof(BigBuffer, totalsBuffer), sizeof(BigBuffer::totalsBuffer));
    glDispatchCompute(GROUP_COUNT, 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    glUseProgram(m_scan2ComputeShader);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, m_bigBuffer, offsetof(BigBuffer, totalsBuffer), sizeof(BigBuffer::totalsBuffer) + sizeof(BigBuffer::totalsTempBuffer));
    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    glUseProgram(m_scan3ComputeShader);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, m_bigBuffer, offsetof(BigBuffer, scanBuffer), sizeof(BigBuffer::scanBuffer));
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, m_bigBuffer, offsetof(BigBuffer, totalsBuffer), sizeof(BigBuffer::totalsBuffer));
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 2, m_bigBuffer, offsetof(BigBuffer, drawCommand), sizeof(BigBuffer::drawCommand));
    glDispatchCompute(GROUP_COUNT, 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    // III/ compact
    glUseProgram(m_compactComputeShader);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_instancesBuffer);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, m_bigBuffer, offsetof(BigBuffer, voteBuffer), sizeof(BigBuffer::voteBuffer));
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 2, m_bigBuffer, offsetof(BigBuffer, scanBuffer), sizeof(BigBuffer::scanBuffer));
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_grassMesh.m_instanceVBO.getId());
    glDispatchCompute(GROUP_COUNT, 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    float theta = 3.14f - renderCamera.getYaw();
    float c = cos(theta), s = sin(theta);
    glm::mat2 facingCameraRotationMatrix = glm::mat2(c, s, -s, c);

    m_grassShader.bind();
    m_grassShader.setUniformMat4f("u_VP", renderCamera.getViewProjectionMatrix());
    m_grassShader.setUniformMat2f("u_R", facingCameraRotationMatrix);
    
    m_grassMesh.m_vao.bind();
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_bigBuffer);
    glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr);
    m_grassMesh.m_vao.unbind();
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