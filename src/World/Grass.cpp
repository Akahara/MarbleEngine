#include "Grass.h"

using namespace Grass;

// TODO profile, the current grass impl is quite slow

GrassChunks::GrassChunks()
  : m_ldInstanceBuffer(0), m_hdInstanceBuffer(0), m_terrain(nullptr), m_currentCameraChunk(), m_ldGrassChunks{}, m_hdGrassChunks{}
{
}

GrassChunks::GrassChunks(const TerrainMeshGenerator::Terrain &terrain, glm::ivec2 creationGrassChunk)
  : m_terrain(&terrain), m_currentCameraChunk()
{
  glGenBuffers(1, &m_hdInstanceBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, m_hdInstanceBuffer);
  glBufferData(GL_ARRAY_BUFFER, BUFFER_CHUNK_SLOT_SIZE * HD_CHUNKS.size(), nullptr, GL_DYNAMIC_DRAW);
  glGenBuffers(1, &m_ldInstanceBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, m_ldInstanceBuffer);
  glBufferData(GL_ARRAY_BUFFER, BUFFER_CHUNK_SLOT_SIZE * LD_CHUNKS.size(), nullptr, GL_DYNAMIC_DRAW);

  size_t i;
  InstanceData *instanceBuffer;

  i = 0;
  glBindBuffer(GL_ARRAY_BUFFER, m_hdInstanceBuffer);
  for (glm::ivec2 offset : HD_CHUNKS) {
    instanceBuffer = (InstanceData *)glMapBufferRange(GL_ARRAY_BUFFER, BUFFER_CHUNK_SLOT_SIZE * i, BUFFER_CHUNK_SLOT_SIZE, GL_MAP_WRITE_BIT);
    fillGrassChunkBuffer(creationGrassChunk + offset, instanceBuffer);
    m_hdGrassChunks[i++] = creationGrassChunk + offset;
    glUnmapBuffer(GL_ARRAY_BUFFER);
  }

  i = 0;
  glBindBuffer(GL_ARRAY_BUFFER, m_ldInstanceBuffer);
  for (glm::ivec2 offset : LD_CHUNKS) {
    instanceBuffer = (InstanceData *)glMapBufferRange(GL_ARRAY_BUFFER, BUFFER_CHUNK_SLOT_SIZE * i, BUFFER_CHUNK_SLOT_SIZE, GL_MAP_WRITE_BIT);
    fillGrassChunkBuffer(creationGrassChunk + offset, instanceBuffer);
    m_ldGrassChunks[i++] = creationGrassChunk + offset;
    glUnmapBuffer(GL_ARRAY_BUFFER);
  }
}

GrassChunks::GrassChunks(GrassChunks &&moved) noexcept
{
  m_hdGrassChunks = std::move(moved.m_hdGrassChunks);
  m_ldGrassChunks = std::move(moved.m_ldGrassChunks);
  m_currentCameraChunk = moved.m_currentCameraChunk;
  m_hdInstanceBuffer = moved.m_hdInstanceBuffer;
  m_ldInstanceBuffer = moved.m_ldInstanceBuffer;
  m_terrain = moved.m_terrain;
  moved.m_hdInstanceBuffer = 0;
  moved.m_ldInstanceBuffer = 0;
}

GrassChunks &GrassChunks::operator=(GrassChunks &&moved) noexcept
{
  this->~GrassChunks();
  new (this)GrassChunks(std::move(moved));
  return *this;
}

GrassChunks::~GrassChunks()
{
  glDeleteBuffers(1, &m_hdInstanceBuffer);
  glDeleteBuffers(1, &m_ldInstanceBuffer);
}

void GrassChunks::step(const Renderer::Camera &camera)
{
  glm::vec2 cameraGrassPosition{ camera.getPosition().x / GRASS_CHUNK_SIZE, camera.getPosition().z / GRASS_CHUNK_SIZE };
  if (glm::compMax(glm::abs(cameraGrassPosition - ((glm::vec2)m_currentCameraChunk + .5f))) > .5f + CAMERA_MOVEMENT_THRESHOLD) {
    m_currentCameraChunk = glm::floor(cameraGrassPosition);
    repopulateGrassChunks();
  }
}

void GrassChunks::repopulateGrassChunks()
{
  repopulateGrassChunks(m_hdGrassChunks, HD_CHUNKS, m_hdInstanceBuffer);
  repopulateGrassChunks(m_ldGrassChunks, LD_CHUNKS, m_ldInstanceBuffer);
}

void GrassChunks::fillGrassChunkBuffer(glm::ivec2 chunkPos, InstanceData *instanceBuffer)
{
  constexpr size_t slotSize = sizeof(InstanceData) * BLADES_PER_CHUNK;
  for (size_t b = 0; b < BLADES_PER_CHUNK; b++) {
    float r = b + chunkPos.x * .252f + chunkPos.y * .62f;
    float bladeX = (chunkPos.x + Mathf::rand(r)) * GRASS_CHUNK_SIZE;
    float bladeZ = (chunkPos.y + Mathf::rand(-r + 2.4f)) * GRASS_CHUNK_SIZE;
    //float bladeY = m_terrain->getHeight(bladeX, bladeZ);
    float bladeY = m_terrain->getHeight(Mathf::positiveModulo(bladeX, CHUNK_SIZE * CHUNK_COUNT_X - 2), Mathf::positiveModulo(bladeZ, CHUNK_SIZE * CHUNK_COUNT_Y - 2));
    float bladeHeight = 1.f + Mathf::fract(r * 634.532f) * .5f;
    InstanceData &blade = instanceBuffer[b];
    blade.position = { bladeX, bladeY, bladeZ, bladeHeight };
  }
}

template<size_t CC>
inline void GrassChunks::repopulateGrassChunks(std::array<glm::ivec2, CC> &currentGrassChunks, const std::array<glm::ivec2, CC> &chunksOffsets, unsigned int instanceBuffer)
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

  // actually regenerate the gpu grass buffer
  glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
  for (i = 0; i < CC; i++) {

    if (newChunkIndices[i] != -1) {
      InstanceData *instanceBufferMemory = (InstanceData *)glMapBufferRange(GL_ARRAY_BUFFER, BUFFER_CHUNK_SLOT_SIZE * newChunkIndices[i], BUFFER_CHUNK_SLOT_SIZE, GL_MAP_WRITE_BIT);
      fillGrassChunkBuffer(newChunks[i], instanceBufferMemory);
      glUnmapBuffer(GL_ARRAY_BUFFER);
    }
  }
}

GrassRenderer::GrassRenderer()
  : m_bigBuffer(0), m_voteComputeShader(0), m_scan1ComputeShader(0), m_scan2ComputeShader(0), m_scan3ComputeShader(0), m_compactComputeShader(0)
{
}

GrassRenderer::GrassRenderer(const TerrainMeshGenerator::Terrain &terrain, glm::ivec2 creationGrassChunk)
  : m_grassChunks(terrain, creationGrassChunk)
{
  m_instanceBuffer = Renderer::VertexBufferObject(TOTAL_BLADE_COUNT * sizeof(Grass::GrassInstance));
  generateGrassModels();
  m_vao.addInstanceBuffer(m_instanceBuffer, Grass::GrassInstance::getLayout(), m_grassModels.getLayout());

  m_grassShader = Renderer::loadShaderFromFiles("res/shaders/grass/grass.vs", "res/shaders/grass/grass.fs");

  m_voteComputeShader = createComputeShader("res/shaders/grass/vote.comp");
  m_scan1ComputeShader = createComputeShader("res/shaders/grass/scan_blocks.comp");
  m_scan2ComputeShader = createComputeShader("res/shaders/grass/scan_groups.comp");
  m_scan3ComputeShader = createComputeShader("res/shaders/grass/scan_accumulate.comp");
  m_compactComputeShader = createComputeShader("res/shaders/grass/compact.comp");

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

GrassRenderer::GrassRenderer(GrassRenderer &&moved) noexcept
{
  m_grassShader = std::move(moved.m_grassShader);
  m_grassModels = std::move(moved.m_grassModels);
  m_grassChunks = std::move(moved.m_grassChunks);
  m_instanceBuffer = std::move(moved.m_instanceBuffer);
  m_vao = std::move(moved.m_vao);
  m_voteComputeShader = moved.m_voteComputeShader;
  m_scan1ComputeShader = moved.m_scan1ComputeShader;
  m_scan2ComputeShader = moved.m_scan2ComputeShader;
  m_scan3ComputeShader = moved.m_scan3ComputeShader;
  m_compactComputeShader = moved.m_compactComputeShader;
  m_bigBuffer = moved.m_bigBuffer;
  moved.m_voteComputeShader = 0;
  moved.m_scan1ComputeShader = 0;
  moved.m_scan2ComputeShader = 0;
  moved.m_scan3ComputeShader = 0;
  moved.m_compactComputeShader = 0;
  moved.m_bigBuffer = 0;
}

GrassRenderer &GrassRenderer::operator=(GrassRenderer &&moved) noexcept
{
  this->~GrassRenderer();
  new (this)GrassRenderer(std::move(moved));
  return *this;
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

void GrassRenderer::step(const Renderer::Camera &camera)
{
  m_grassChunks.step(camera);
}

void GrassRenderer::render(const Renderer::Camera &camera, const Renderer::Camera &frustumCamera, float time)
{
  render(camera, frustumCamera, time, m_grassChunks.getLDInstanceBuffer(), 1);
  render(camera, frustumCamera, time, m_grassChunks.getHDInstanceBuffer(), 0);
}

unsigned int GrassRenderer::createComputeShader(const char *sourcePath)
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

void GrassRenderer::generateGrassModels()
{
  float l = .1f;
  float s = .07f;
  float k = .03f;
  float h = .5f;
  float m = .85f;
  float t = 1.f;

  { // HD
    std::vector<Grass::GrassModelVertex> hdBladeVertices;
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
    std::vector<Grass::GrassModelVertex> ldBladeVertices;
    std::vector<unsigned int> ldBladeIndices;
    ldBladeVertices.push_back({ {-l, 0, 0} });
    ldBladeVertices.push_back({ {+l, 0, 0} });
    ldBladeVertices.push_back({ {0, t, 0} });
    ldBladeIndices.push_back(0); ldBladeIndices.push_back(2); ldBladeIndices.push_back(1);

    m_grassModels.emplaceBuffer(1, ldBladeVertices, ldBladeIndices);
  }
}

void GrassRenderer::render(const Renderer::Camera &camera, const Renderer::Camera &frustumCamera, float time, int instanceBuffer, int lod)
{
  // I/ vote
  glm::mat4 V = frustumCamera.getViewMatrix();
  V = glm::translate(V, frustumCamera.getForward() * 1.f); // move the clip camera back a bit to be sure blades that are very close to the actual camera do not get clipped
  glm::mat4 VP = frustumCamera.getProjectionMatrix() * V;
  glUseProgram(m_voteComputeShader);
  glUniformMatrix4fv(glGetUniformLocation(m_voteComputeShader, "u_VP"), 1, GL_FALSE, glm::value_ptr(VP));
  glUniform1ui(glGetUniformLocation(m_voteComputeShader, "N"), BLADES_PER_CHUNK * 9); // TODO temporary 9, change depending on the lod (depending on the chunk count), this prevents a part of ld grass from rendering
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, instanceBuffer);
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
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, instanceBuffer);
  glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, m_bigBuffer, offsetof(BigBuffer, voteBuffer), sizeof(BigBuffer::voteBuffer));
  glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 2, m_bigBuffer, offsetof(BigBuffer, scanBuffer), sizeof(BigBuffer::scanBuffer));
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_instanceBuffer.getId());
  glDispatchCompute(GROUP_COUNT, 1, 1);
  glMemoryBarrier(GL_ALL_BARRIER_BITS);

  float theta = 3.14f - camera.getYaw();
  float c = cos(theta), s = sin(theta);
  glm::mat2 facingCameraRotationMatrix = glm::mat2(c, s, -s, c);

  m_grassShader.bind();
  m_grassShader.setUniformMat4f("u_VP", camera.getViewProjectionMatrix());
  m_grassShader.setUniformMat2f("u_R", facingCameraRotationMatrix);
  m_grassShader.setUniform1f("u_time", time);

  m_vao.bind();
  m_grassModels.bindBuffer(lod, m_vao);
  int verticesCount = (int)m_grassModels.getIndexBuffer(lod).getCount();
  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_bigBuffer);
  glBufferSubData(GL_DRAW_INDIRECT_BUFFER, offsetof(BigBuffer, drawCommand) + offsetof(Renderer::IndirectDrawCommand, count), sizeof(int), &verticesCount);
  glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr);
  Renderer::VertexArray::unbind();
}
