#pragma once

#include <array>
#include <fstream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/component_wise.hpp>

#include "../Utils/Mathf.h"
#include "../Utils/MathIterators.h"
#include "../Utils/StringUtils.h"
#include "../World/TerrainGeneration/MeshGenerator.h"
#include "../abstraction/InstancedRenderer.h"
#include "../abstraction/Camera.h"
#include "../abstraction/Shader.h"
#include "../abstraction/UnifiedRenderer.h"

static constexpr unsigned int CHUNK_COUNT_X = 5, CHUNK_COUNT_Y = 5, CHUNK_SIZE = 25; // TODO this cannot be removed until the terrain is refactored
                                                                                       // the terrain must be able to be sampled outside of the cached region, (ie. currently sampling at -1,-1 crashes)

namespace Grass {

static constexpr unsigned int GROUP_COUNT = 256, GROUP_WORK = 1024;
static constexpr int MAX_BLADE_COUNT_PER_DRAWCALL = GROUP_COUNT * GROUP_WORK;

struct GrassModelVertex {
  glm::vec3 position; // model position

  static Renderer::VertexBufferLayout getLayout()
  {
    Renderer::VertexBufferLayout layout{};
    layout.push<float>(3);
    return layout;
  }
};

struct GrassInstance {
  glm::vec4 position;

  static Renderer::VertexBufferLayout getLayout()
  {
    Renderer::VertexBufferLayout layout{};
    layout.push<float>(4);
    return layout;
  }
};

}

constexpr auto getHDChunks()
{
  constexpr auto chunksIterator = Iterators::iterateOverSquare({ 0,0 }, 3); // 3x3 chunks
  std::array<glm::ivec2, chunksIterator.getCount()> chunks{};
  size_t i = 0;
  for (glm::ivec2 c : chunksIterator)
    chunks[i++] = c;
  return chunks;
}

constexpr auto getLDChunks()
{
  constexpr auto chunksIterator = Iterators::iterateOverDiamond({ 0,0 }, 4);
  constexpr auto hdChunks = getHDChunks();
  constexpr size_t ldChunkCount = chunksIterator.getCount() - hdChunks.size(); // assumes that chunksIterator completely overlaps hdChunks
  std::array<glm::ivec2, ldChunkCount> chunks{};
  size_t i = 0;
  for (glm::ivec2 c : chunksIterator) {
    if (std::find(hdChunks.begin(), hdChunks.end(), c) == hdChunks.end())
      chunks[i++] = c;
  }
  // there is no assertion mechanism that can be used here
  // to validate that all the chunks have been filled
  return chunks;
}

class GrassChunks {
private:
  using InstanceData = Grass::GrassInstance;

public:
  static constexpr auto HD_CHUNKS = getHDChunks();
  static constexpr auto LD_CHUNKS = getLDChunks();
  static constexpr float GRASS_CHUNK_SIZE = 50;
  static constexpr float CAMERA_MOVEMENT_THRESHOLD = .2f; // the grass chunks will be regenerated when the camera moves by more than the threshold into another chunk
  static constexpr int BLADES_PER_HD_CHUNK = Grass::MAX_BLADE_COUNT_PER_DRAWCALL / HD_CHUNKS.size();
  static constexpr int BLADES_PER_LD_CHUNK = Grass::MAX_BLADE_COUNT_PER_DRAWCALL / LD_CHUNKS.size();
private:
  static constexpr size_t BUFFER_HD_CHUNK_SLOT_SIZE = sizeof(InstanceData) * BLADES_PER_HD_CHUNK;
  static constexpr size_t BUFFER_LD_CHUNK_SLOT_SIZE = sizeof(InstanceData) * BLADES_PER_LD_CHUNK;
  std::array<glm::ivec2, HD_CHUNKS.size()> m_hdGrassChunks;
  std::array<glm::ivec2, LD_CHUNKS.size()> m_ldGrassChunks;
  glm::ivec2                               m_currentCameraChunk;
  const TerrainMeshGenerator::Terrain     *m_terrain;
  unsigned int                             m_hdInstanceBuffer;
  unsigned int                             m_ldInstanceBuffer;

public:
  GrassChunks();
  GrassChunks(const TerrainMeshGenerator::Terrain &terrain, glm::ivec2 creationGrassChunk);
  GrassChunks(const GrassChunks &) = delete;
  GrassChunks &operator=(const GrassChunks &) = delete;
  GrassChunks(GrassChunks &&moved) noexcept;
  GrassChunks &operator=(GrassChunks &&moved) noexcept;

  ~GrassChunks();

  void step(const Renderer::Camera &camera);

  int getHDInstanceBuffer() const { return m_hdInstanceBuffer; }
  int getLDInstanceBuffer() const { return m_ldInstanceBuffer; }

  static inline glm::ivec2 positionToGrassChunk(const glm::vec3 &position)
  {
    return { glm::floor(position.x / GRASS_CHUNK_SIZE), glm::floor(position.z / GRASS_CHUNK_SIZE) };
  }

private:
  void repopulateGrassChunks();
  template<size_t CC>
  void repopulateGrassChunks(std::array<glm::ivec2, CC> &currentGrassChunks, const std::array<glm::ivec2, CC> &chunksOffsets, unsigned int instanceBuffer, unsigned int instanceCountPerBufferSlot);

  void fillGrassChunkBuffer(glm::ivec2 chunkPos, InstanceData *instanceBuffer, size_t instanceCount);
};

class GrassRenderer {
private:
  Renderer::Shader m_grassShader;
  Renderer::VertexArray m_vao;
  Renderer::VertexBufferObject m_instanceBuffer;
  Renderer::VertexBufferHolder<Grass::GrassModelVertex, 2> m_grassModels;
  GrassChunks  m_grassChunks;
  unsigned int m_voteComputeShader;
  unsigned int m_scan1ComputeShader;
  unsigned int m_scan2ComputeShader;
  unsigned int m_scan3ComputeShader;
  unsigned int m_compactComputeShader;
  unsigned int m_bigBuffer;

  /*
   * This structure is never actually instantiated on the cpu be will be on the GPU, and the layout will remain the same.
   * It contains all the data space necessary for the vote-scan&compact algorithms.
   */
  struct BigBuffer {
    Renderer::IndirectDrawCommand drawCommand; // must be the first member because BigBuffer will be used as a GL_DRAW_INDIRECT_BUFFER
    int voteBuffer[Grass::MAX_BLADE_COUNT_PER_DRAWCALL];
    int scanBuffer[Grass::MAX_BLADE_COUNT_PER_DRAWCALL];
    int scanTempBuffer[Grass::MAX_BLADE_COUNT_PER_DRAWCALL];
    int totalsBuffer[Grass::GROUP_COUNT];
    int totalsTempBuffer[Grass::GROUP_COUNT];
  };
public:
  GrassRenderer();
  GrassRenderer(const TerrainMeshGenerator::Terrain &terrain, glm::ivec2 creationGrassChunk);
  GrassRenderer(const GrassRenderer &) = delete;
  GrassRenderer& operator=(const GrassRenderer &) = delete;
  GrassRenderer(GrassRenderer &&moved) noexcept;
  GrassRenderer &operator=(GrassRenderer &&moved) noexcept;
  ~GrassRenderer();

  void step(const Renderer::Camera &camera);

  void render(const Renderer::Camera &camera, float time) { render(camera, camera, time); }
  void render(const Renderer::Camera &camera, const Renderer::Camera &frustumCamera, float time);
  
private:
  static unsigned int createComputeShader(const char *sourcePath); // TODO move elsewhere

  void generateGrassModels();

  void render(const Renderer::Camera &camera, const Renderer::Camera &frustumCamera, float time, int instanceBuffer, int lod);
};
