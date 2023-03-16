#include "Grass.h"

#include <fstream>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/component_wise.hpp>

#include "../Utils/StringUtils.h"
#include "../abstraction/Shader.h"
#include "../abstraction/SpecializedRender.h"
#include "../abstraction/UnifiedRenderer.h"

namespace World {

struct GrassModelVertex {
  glm::vec3 position; // model position

  static Renderer::VertexBufferLayout getLayout()
  {
    Renderer::VertexBufferLayout layout{};
    layout.push<float>(3);
    return layout;
  }

  // Beware! This method should be in the GrassInstance class
  // but is here to avoid dependencies in the .h file, if
  // GrassInstance is updated this method MUST change!
  static Renderer::VertexBufferLayout getInstanceLayout()
  {
    Renderer::VertexBufferLayout layout{};
    layout.push<float>(4);
    return layout;
  }
};

class GrassRenderer {
private:
  Renderer::Shader m_grassShader;
  Renderer::VertexArray m_vao;
  Renderer::VertexBufferObject m_instanceBuffer;
  Renderer::VertexBufferHolder<GrassModelVertex, 2> m_grassModels;
  unsigned int m_voteComputeShader;
  unsigned int m_scan1ComputeShader;
  unsigned int m_scan2ComputeShader;
  unsigned int m_scan3ComputeShader;
  unsigned int m_compactComputeShader;
  unsigned int m_bigBuffer;

  /*
   * This structure is never actually instantiated on the cpu but will be on the GPU,
   * and the layout will remain the same. It contains all the data space necessary
   * for the vote-scan&compact algorithms.
   */
  struct BigBuffer {
    Renderer::IndirectDrawCommand drawCommand; // must be the first member because BigBuffer will be used as a GL_DRAW_INDIRECT_BUFFER
    int voteBuffer[GrassRenderSettings::MAX_BLADE_COUNT_PER_DRAWCALL];
    int scanBuffer[GrassRenderSettings::MAX_BLADE_COUNT_PER_DRAWCALL];
    int scanTempBuffer[GrassRenderSettings::MAX_BLADE_COUNT_PER_DRAWCALL];
    int totalsBuffer[GrassRenderSettings::GROUP_COUNT];
    int totalsTempBuffer[GrassRenderSettings::GROUP_COUNT];
  };
public:
  GrassRenderer();
  ~GrassRenderer();
  GrassRenderer(const GrassRenderer &) = delete;
  GrassRenderer &operator=(const GrassRenderer &) = delete;

  void render(const Renderer::Camera &camera, const Renderer::Camera &frustumCamera, float time, const WorldGrass &grass);

private:
  static unsigned int loadComputeShader(const char *sourcePath);

  void generateGrassModels();

  void renderSingleLOD(const Renderer::Camera &camera, const Renderer::Camera &frustumCamera, float time, unsigned int instanceBuffer, size_t instanceCount, int lod);
};

}

namespace World {

WorldGrass::WorldGrass(unsigned int hdInstanceCount, unsigned int ldInstanceCount)
  : m_hdInstanceCount(hdInstanceCount), m_ldInstanceCount(ldInstanceCount)
{
  glGenBuffers(1, &m_hdInstanceBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, m_hdInstanceBuffer);
  glBufferData(GL_ARRAY_BUFFER, GrassRenderSettings::MAX_BLADE_COUNT_PER_DRAWCALL * sizeof(GrassInstance), nullptr, GL_DYNAMIC_DRAW);
  glGenBuffers(1, &m_ldInstanceBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, m_ldInstanceBuffer);
  glBufferData(GL_ARRAY_BUFFER, GrassRenderSettings::MAX_BLADE_COUNT_PER_DRAWCALL * sizeof(GrassInstance), nullptr, GL_DYNAMIC_DRAW);
}

WorldGrass::~WorldGrass()
{
  glDeleteBuffers(1, &m_hdInstanceBuffer);
  glDeleteBuffers(1, &m_ldInstanceBuffer);
}

TerrainGrass::TerrainGrass(std::unique_ptr<WorldGrass> &&world)
//: m_world(std::move(world)), m_renderer(std::make_unique<GrassRenderer>())
: m_world(std::move(world)), m_renderer(new GrassRenderer())
{
  m_world->regenerate();
}

TerrainGrass::TerrainGrass(TerrainGrass &&moved) noexcept
{
  m_world = std::move(moved.m_world);
  m_renderer = moved.m_renderer;
  moved.m_renderer = nullptr;
}

TerrainGrass &TerrainGrass::operator=(TerrainGrass &&moved) noexcept
{
  this->~TerrainGrass();
  new (this) TerrainGrass(std::move(moved));
  return *this;
}

TerrainGrass::~TerrainGrass()
{
  delete m_renderer;
}

void TerrainGrass::render(const Renderer::Camera &camera, const Renderer::Camera &frustumCamera, float time)
{
  m_renderer->render(camera, frustumCamera, time, *m_world);
}

void TerrainGrassGenerator::regenerateChunk(const glm::ivec2 &chunkPosition, unsigned int chunkSize, size_t instanceCount, GrassInstance *grassBuffer)
{
  size_t slotSize = sizeof(GrassInstance) * instanceCount;
  for (size_t b = 0; b < instanceCount; b++) {
    float r = b + chunkPosition.x * .252f + chunkPosition.y * .62f;
    float bladeX = (chunkPosition.x + Mathf::rand(r)) * chunkSize;
    float bladeZ = (chunkPosition.y + Mathf::rand(-r + 2.4f)) * chunkSize;
    float bladeY = (*m_heightmap)(bladeX, bladeZ);
    float bladeHeight = 1.f + Mathf::fract(r * 634.532f) * .5f;
    GrassInstance &blade = grassBuffer[b];
    blade.position = { bladeX, bladeY, bladeZ, bladeHeight };
  }
}

void InfiniteGrassWorld::regenerate()
{
  size_t i;
  GrassInstance *instanceBuffer;
  unsigned int bufferHDChunkSlotSize = m_bladesPerHDChunk * sizeof(GrassInstance);
  unsigned int bufferLDChunkSlotSize = m_bladesPerLDChunk * sizeof(GrassInstance);

  i = 0;
  glBindBuffer(GL_ARRAY_BUFFER, m_hdInstanceBuffer);
  for (glm::ivec2 offset : HD_CHUNKS) {
    instanceBuffer = (GrassInstance *)glMapBufferRange(GL_ARRAY_BUFFER, bufferHDChunkSlotSize * i, bufferHDChunkSlotSize, GL_MAP_WRITE_BIT);
    m_generator->regenerateChunk(m_currentCameraChunk + offset, m_chunkSize, m_bladesPerHDChunk, instanceBuffer);
    m_hdGrassChunks[i++] = m_currentCameraChunk + offset;
    glUnmapBuffer(GL_ARRAY_BUFFER);
  }

  i = 0;
  glBindBuffer(GL_ARRAY_BUFFER, m_ldInstanceBuffer);
  for (glm::ivec2 offset : LD_CHUNKS) {
    instanceBuffer = (GrassInstance *)glMapBufferRange(GL_ARRAY_BUFFER, bufferLDChunkSlotSize * i, bufferLDChunkSlotSize, GL_MAP_WRITE_BIT);
    m_generator->regenerateChunk(m_currentCameraChunk + offset, m_chunkSize, m_bladesPerLDChunk, instanceBuffer);
    m_ldGrassChunks[i++] = m_currentCameraChunk + offset;
    glUnmapBuffer(GL_ARRAY_BUFFER);
  }
}

void InfiniteGrassWorld::step(const Renderer::Camera &camera)
{
  glm::vec2 cameraGrassPosition{ camera.getPosition().x / m_chunkSize, camera.getPosition().z / m_chunkSize };
  if (glm::compMax(glm::abs(cameraGrassPosition - ((glm::vec2)m_currentCameraChunk + .5f))) > .5f + CAMERA_MOVEMENT_THRESHOLD) {
    m_currentCameraChunk = glm::floor(cameraGrassPosition);
    repopulateGrassChunks(m_hdGrassChunks, HD_CHUNKS, m_hdInstanceBuffer, m_bladesPerHDChunk);
    repopulateGrassChunks(m_ldGrassChunks, LD_CHUNKS, m_ldInstanceBuffer, m_bladesPerLDChunk);
  }
}

template<size_t CC>
inline void InfiniteGrassWorld::repopulateGrassChunks(std::array<glm::ivec2, CC> &currentGrassChunks, const std::array<glm::ivec2, CC> &chunksOffsets, unsigned int instanceBuffer, unsigned int instanceCountPerBufferSlot)
{
  std::array<glm::ivec2, CC> newChunks{};
  size_t i = 0;
  for (const glm::ivec2 &offset : chunksOffsets)
    newChunks[i++] = m_currentCameraChunk + offset;

  std::array<size_t, CC> newChunkIndices{};
  for (i = 0; i < CC; i++) {
    if (std::find(currentGrassChunks.begin(), currentGrassChunks.end(), newChunks[i]) == currentGrassChunks.end()) {
      // a new chunk, insert it in the first slot available
      size_t availableIndex = 0;
      while (std::find(newChunks.begin(), newChunks.end(), currentGrassChunks[availableIndex]) != newChunks.end())
        availableIndex++;
      currentGrassChunks[availableIndex] = newChunks[i];
      newChunkIndices[i] = availableIndex;
    } else {
      newChunkIndices[i] = -1;
    }
  }

  size_t bufferSlotSize = instanceCountPerBufferSlot * sizeof(GrassInstance);

  // actually regenerate the gpu grass buffer
  glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
  for (i = 0; i < CC; i++) {
    if (newChunkIndices[i] != -1) {
      GrassInstance *instanceBufferMemory = (GrassInstance *)glMapBufferRange(GL_ARRAY_BUFFER, bufferSlotSize * newChunkIndices[i], bufferSlotSize, GL_MAP_WRITE_BIT);
      m_generator->regenerateChunk(newChunks[i], m_chunkSize, instanceCountPerBufferSlot, instanceBufferMemory);
      glUnmapBuffer(GL_ARRAY_BUFFER);
    }
  }
}

void FixedGrassChunks::regenerate()
{
  size_t i;
  GrassInstance *instanceBuffer;
  unsigned int bladesPerHDChunk = GrassRenderSettings::MAX_BLADE_COUNT_PER_DRAWCALL / (unsigned int)m_hdChunks.size();
  unsigned int bladesPerLDChunk = GrassRenderSettings::MAX_BLADE_COUNT_PER_DRAWCALL / (unsigned int)m_ldChunks.size();
  unsigned int bufferHDChunkSlotSize = bladesPerHDChunk * sizeof(GrassInstance);
  unsigned int bufferLDChunkSlotSize = bladesPerLDChunk * sizeof(GrassInstance);

  i = 0;
  glBindBuffer(GL_ARRAY_BUFFER, m_hdInstanceBuffer);
  for (glm::ivec2 position : m_hdChunks) {
    instanceBuffer = (GrassInstance *)glMapBufferRange(GL_ARRAY_BUFFER, bufferHDChunkSlotSize * i, bufferHDChunkSlotSize, GL_MAP_WRITE_BIT);
    m_generator->regenerateChunk(position, m_chunkSize, bladesPerHDChunk, instanceBuffer);
    glUnmapBuffer(GL_ARRAY_BUFFER);
    i++;
  }

  i = 0;
  glBindBuffer(GL_ARRAY_BUFFER, m_ldInstanceBuffer);
  for (glm::ivec2 position : m_ldChunks) {
    instanceBuffer = (GrassInstance *)glMapBufferRange(GL_ARRAY_BUFFER, bufferLDChunkSlotSize * i, bufferLDChunkSlotSize, GL_MAP_WRITE_BIT);
    m_generator->regenerateChunk(position, m_chunkSize, bladesPerLDChunk, instanceBuffer);
    glUnmapBuffer(GL_ARRAY_BUFFER);
    i++;
  }
}

GrassRenderer::GrassRenderer()
{
  m_instanceBuffer = Renderer::VertexBufferObject(nullptr, GrassRenderSettings::MAX_BLADE_COUNT_PER_DRAWCALL * sizeof(GrassInstance));
  generateGrassModels();
  m_vao.addInstanceBuffer(m_instanceBuffer, GrassModelVertex::getInstanceLayout(), m_grassModels.getLayout());

  m_grassShader = std::move(*Renderer::loadShaderFromFiles("res/shaders/grass/grass.vs", "res/shaders/grass/grass.fs"));

  m_voteComputeShader = loadComputeShader("res/shaders/grass/vote.comp");
  m_scan1ComputeShader = loadComputeShader("res/shaders/grass/scan_blocks.comp");
  m_scan2ComputeShader = loadComputeShader("res/shaders/grass/scan_groups.comp");
  m_scan3ComputeShader = loadComputeShader("res/shaders/grass/scan_accumulate.comp");
  m_compactComputeShader = loadComputeShader("res/shaders/grass/compact.comp");

  Renderer::IndirectDrawCommand filledDrawCommand{};
  filledDrawCommand.count = -1; // to be set before each draw call (depending on the lod)
  filledDrawCommand.firstIndex = 0;
  filledDrawCommand.instanceCount = -1; // to be filled by compute shaders
  filledDrawCommand.baseInstance = 0;
  filledDrawCommand.baseVertex = 0;

  glCreateBuffers(1, &m_bigBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, m_bigBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(BigBuffer), nullptr, GL_DYNAMIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, offsetof(BigBuffer, drawCommand), sizeof(BigBuffer::drawCommand), &filledDrawCommand); // fill only the drawCommand part of the big buffer
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

GrassRenderer::~GrassRenderer()
{
  glDeleteProgram(m_voteComputeShader);
  glDeleteProgram(m_scan1ComputeShader);
  glDeleteProgram(m_scan2ComputeShader);
  glDeleteProgram(m_scan3ComputeShader);
  glDeleteProgram(m_compactComputeShader);
  glDeleteBuffers(1, &m_bigBuffer);
}

void GrassRenderer::render(const Renderer::Camera &camera, const Renderer::Camera &frustumCamera, float time, const WorldGrass &grass)
{
  renderSingleLOD(camera, frustumCamera, time, grass.getHDInstanceBuffer(), grass.getHDInstanceCount(), 0);
  renderSingleLOD(camera, frustumCamera, time, grass.getLDInstanceBuffer(), grass.getLDInstanceCount(), 1);
}

unsigned int GrassRenderer::loadComputeShader(const char *sourcePath)
{
  std::ifstream vertexFile{ sourcePath };
  std::stringstream buffer;
  buffer << vertexFile.rdbuf();
  std::string vertexCode = buffer.str();
  Utils::replaceAll(vertexCode, "{WORK_SIZE}", std::to_string(GrassRenderSettings::GROUP_WORK));
  Utils::replaceAll(vertexCode, "{GROUP_COUNT}", std::to_string(GrassRenderSettings::GROUP_COUNT));

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

void GrassRenderer::generateGrassModels()
{
  float l = .1f;
  float s = .07f;
  float k = .03f;
  float h = .5f;
  float m = .85f;
  float t = 1.f;

  { // HD
    std::vector<GrassModelVertex> hdBladeVertices;
    std::vector<unsigned int> hdBladeIndices;
    hdBladeVertices.push_back({ {-l,0,0} });
    hdBladeVertices.push_back({ {l,0,0} });
    hdBladeVertices.push_back({ {-s,h,0} });
    hdBladeVertices.push_back({ {s,h,0} });
    hdBladeVertices.push_back({ {-k,m,0} });
    hdBladeVertices.push_back({ {k,m,0} });
    hdBladeVertices.push_back({ {0,t,0} });
    hdBladeIndices.push_back(0); hdBladeIndices.push_back(2); hdBladeIndices.push_back(1);
    hdBladeIndices.push_back(1); hdBladeIndices.push_back(2); hdBladeIndices.push_back(3);
    hdBladeIndices.push_back(2); hdBladeIndices.push_back(4); hdBladeIndices.push_back(3);
    hdBladeIndices.push_back(3); hdBladeIndices.push_back(4); hdBladeIndices.push_back(5);
    hdBladeIndices.push_back(5); hdBladeIndices.push_back(4); hdBladeIndices.push_back(6);

    m_grassModels.emplaceBuffer(0, hdBladeVertices, hdBladeIndices);
  }

  { // LD
    std::vector<GrassModelVertex> ldBladeVertices;
    std::vector<unsigned int> ldBladeIndices;
    ldBladeVertices.push_back({ {-l, 0, 0} });
    ldBladeVertices.push_back({ {+l, 0, 0} });
    ldBladeVertices.push_back({ {0, t*1.1f, 0} });
    ldBladeIndices.push_back(0); ldBladeIndices.push_back(2); ldBladeIndices.push_back(1);

    m_grassModels.emplaceBuffer(1, ldBladeVertices, ldBladeIndices);
  }
}

void GrassRenderer::renderSingleLOD(const Renderer::Camera &camera, const Renderer::Camera &frustumCamera, float time, unsigned int instanceBuffer, size_t instanceCount, int lod)
{
  // I/ vote
  glm::mat4 V = frustumCamera.getViewMatrix();
  V = glm::translate(V, frustumCamera.getForward() * 1.f); // move the clip camera back a bit to be sure blades that are very close to the actual camera do not get clipped
  glm::mat4 VP = frustumCamera.getProjectionMatrix() * V;
  glUseProgram(m_voteComputeShader);
  glUniformMatrix4fv(glGetUniformLocation(m_voteComputeShader, "u_VP"), 1, GL_FALSE, glm::value_ptr(VP));
  glUniform1ui(glGetUniformLocation(m_voteComputeShader, "N"), (unsigned int)instanceCount);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, instanceBuffer);
  glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, m_bigBuffer, offsetof(BigBuffer, voteBuffer), sizeof(BigBuffer::voteBuffer));
  glDispatchCompute(GrassRenderSettings::GROUP_COUNT, 1, 1);
  //glMemoryBarrier(GL_ALL_BARRIER_BITS);

  // II/ scan
  glUseProgram(m_scan1ComputeShader);
  glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, m_bigBuffer, offsetof(BigBuffer, voteBuffer), sizeof(BigBuffer::voteBuffer));
  glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, m_bigBuffer, offsetof(BigBuffer, scanBuffer), sizeof(BigBuffer::scanBuffer) + sizeof(BigBuffer::scanTempBuffer));
  glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 2, m_bigBuffer, offsetof(BigBuffer, totalsBuffer), sizeof(BigBuffer::totalsBuffer));
  glDispatchCompute(GrassRenderSettings::GROUP_COUNT, 1, 1);
  //glMemoryBarrier(GL_ALL_BARRIER_BITS);

  glUseProgram(m_scan2ComputeShader);
  glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, m_bigBuffer, offsetof(BigBuffer, totalsBuffer), sizeof(BigBuffer::totalsBuffer) + sizeof(BigBuffer::totalsTempBuffer));
  glDispatchCompute(1, 1, 1);
  //glMemoryBarrier(GL_ALL_BARRIER_BITS);

  glUseProgram(m_scan3ComputeShader);
  glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, m_bigBuffer, offsetof(BigBuffer, scanBuffer), sizeof(BigBuffer::scanBuffer));
  glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, m_bigBuffer, offsetof(BigBuffer, totalsBuffer), sizeof(BigBuffer::totalsBuffer));
  glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 2, m_bigBuffer, offsetof(BigBuffer, drawCommand), sizeof(BigBuffer::drawCommand));
  glDispatchCompute(GrassRenderSettings::GROUP_COUNT, 1, 1);
  //glMemoryBarrier(GL_ALL_BARRIER_BITS);

  // III/ compact
  glUseProgram(m_compactComputeShader);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, instanceBuffer);
  glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, m_bigBuffer, offsetof(BigBuffer, voteBuffer), sizeof(BigBuffer::voteBuffer));
  glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 2, m_bigBuffer, offsetof(BigBuffer, scanBuffer), sizeof(BigBuffer::scanBuffer));
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_instanceBuffer.getId());
  glDispatchCompute(GrassRenderSettings::GROUP_COUNT, 1, 1);
  //glMemoryBarrier(GL_ALL_BARRIER_BITS);

  float theta = 3.14f - camera.getYaw();
  float c = cos(theta), s = sin(theta);
  glm::mat2 facingCameraRotationMatrix = glm::mat2(c, s, -s, c);

  m_grassShader.bind();
  m_grassShader.setUniformMat4f("u_VP", camera.getViewProjectionMatrix());
  m_grassShader.setUniformMat2f("u_R", facingCameraRotationMatrix);
  m_grassShader.setUniform1f("u_time", time);

  m_grassModels.bindBuffer(lod, m_vao);
  m_vao.bind();
  int verticesCount = (int)m_grassModels.getIndexBuffer(lod).getCount();
  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_bigBuffer);
  glBufferSubData(GL_DRAW_INDIRECT_BUFFER, offsetof(BigBuffer, drawCommand) + offsetof(Renderer::IndirectDrawCommand, count), sizeof(int), &verticesCount);
  glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr);
  Renderer::VertexArray::unbind();
}

}