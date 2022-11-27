#pragma once

#include <array>

#include <glm/glm.hpp>

#include "../Utils/Mathf.h"
#include "../Utils/MathIterators.h"
#include "../World/TerrainGeneration/Terrain.h"
#include "../abstraction/Camera.h"

namespace World {

// TODO add a buffer_t type

/* Settings for the scan&compact compute shader algorithm, dictates how many grass blades can be renderered */
struct GrassRenderSettings {
  static constexpr unsigned int GROUP_COUNT = 256;
  static constexpr unsigned int GROUP_WORK = 1024;

  static constexpr unsigned int MAX_BLADE_COUNT_PER_DRAWCALL = GROUP_COUNT * GROUP_WORK;

  static_assert(Mathf::isPowerOfTwo(GROUP_COUNT) && GROUP_COUNT <= 1024);
  static_assert(Mathf::isPowerOfTwo(GROUP_WORK) && GROUP_WORK <= 1024);
};

struct GrassInstance {
  glm::vec4 position;
};

/*
 * Grass generators are responsible for the placement of each and every grass blade.
 * It would be better for implementations to be deterministic. Different calls to
 * #regenerateChunk will have the same chunkSize parameters but possibly different
 * instanceCount values depending on the buffers'LOD.
 * 
 * See below for implementations
 */
class GrassGenerator {
public:
  virtual void regenerateChunk(const glm::ivec2 &chunkPosition, unsigned int chunkSize, size_t instanceCount, GrassInstance *grassBuffer) = 0;
};

/*
 * World grass are responsible for holding references to LD and HD grass buffers.
 * A simple way to hangling the filling of these buffers is to divide them in chunks
 * (of possibly different sizes than the world chunks) and fill the chunks using
 * GrassGenerators.
 * The size of the buffers is specified by GrassRenderSettings#MAX_BLADE_COUNT_PER_DRAWCALL
 * but they do not need to be filled completely. The implementations of #regenerate
 * and #step should be careful not to overrun the buffers' ends.
 * The actual number of used grass blades must be given to the constructor and shall
 * not be changing during execution.
 * 
 * See below for implementations
 */
class WorldGrass {
protected:
  unsigned int m_hdInstanceCount;
  unsigned int m_ldInstanceCount;
  unsigned int m_hdInstanceBuffer;
  unsigned int m_ldInstanceBuffer;
public:
  WorldGrass(unsigned int hdInstanceCount, unsigned int ldInstanceCount);
  ~WorldGrass();

  virtual void regenerate() = 0;
  virtual void step(const Renderer::Camera &camera) = 0;
  inline unsigned int getLDInstanceBuffer() const { return m_ldInstanceBuffer; }
  inline unsigned int getHDInstanceBuffer() const { return m_hdInstanceBuffer; }
  inline unsigned int getLDInstanceCount() const { return m_hdInstanceCount; }
  inline unsigned int getHDInstanceCount() const { return m_ldInstanceCount; }

  WorldGrass(const WorldGrass &) = delete;
  WorldGrass &operator=(const WorldGrass &) = delete;
};

/*
 * The GrassRenderer is responsible for the actual rendering of grass blades.
 * The renderer uses a scan&compact algorithm to determine among instance buffers
 * which instances should be rendered. Each pass of the renderer run on two
 * instance buffers, one in low definition (LD) and one in high definition (HD).
 * 
 * Defined in the cpp file to avoid dependencies.
 */
class GrassRenderer;

/*
 * TerrainGrass is the main class users will be interacting with. It holds a
 * WorldGrass object that holds the ld&hd grass blade instance buffers. It also
 * holds a GrassRenderer to be able to actually draw the blades on screen.
 * 
 * Note that grass rendering is *very* resource intensive, it may not be wise
 * to draw details such as grass during every render pass.
 */
class TerrainGrass {
private:
  std::unique_ptr<WorldGrass>    m_world;
  GrassRenderer *m_renderer; // cannot use a unique_ptr because it would need ~GrassRenderer to be declared

public:
  TerrainGrass() : m_renderer(nullptr) {}
  TerrainGrass(std::unique_ptr<WorldGrass> &&world);
  ~TerrainGrass();

  TerrainGrass(const TerrainGrass &) = delete;
  TerrainGrass &operator=(const TerrainGrass &) = delete;
  TerrainGrass(TerrainGrass &&moved) noexcept;
  TerrainGrass &operator=(TerrainGrass &&moved) noexcept;

  void step(const Renderer::Camera &camera)
  {
    m_world->step(camera);
  }

  void render(const Renderer::Camera &camera, float time) { render(camera, camera, time); }
  void render(const Renderer::Camera &camera, const Renderer::Camera &frustumCamera, float time);
};

}

// Implementations
namespace World {

/*
 * A simple grass generator that places grass blades on top of a terrain
 * using its heightmap. Blades are placed somewhat evenly on chunks.
 * This generator is deterministic.
 */
class TerrainGrassGenerator : public GrassGenerator {
private:
  const Terrain::Terrain *m_terrain;
public:
  TerrainGrassGenerator(const Terrain::Terrain *terrain)
    : m_terrain(terrain) { }

  void regenerateChunk(const glm::ivec2 &chunkPosition, unsigned int chunkSize, size_t instanceCount, GrassInstance *grassBuffer) override;
};


// for some reason these methods cannot be in the InfiniteGrassWorld class, even if static
namespace InfiniteGrassWorldImpl {

static constexpr auto getHDChunksOffsets()
{
  constexpr auto chunksIterator = Iterators::iterateOverSquare({ 0,0 }, 3); // 3x3 chunks
  std::array<glm::ivec2, chunksIterator.getCount()> chunks{};
  size_t i = 0;
  for (glm::ivec2 c : chunksIterator)
    chunks[i++] = c;
  return chunks;
}

static constexpr auto getLDChunksOffsets()
{
  constexpr auto chunksIterator = Iterators::iterateOverDiamond({ 0,0 }, 4);
  constexpr auto hdChunks = getHDChunksOffsets();
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

} // !namespace InfiniteGrassWorldImpl


/*
 * A grass world that updates grass buffers depending on the camera position.
 * When the camera steps too far outside of its current chunk ld&hd chunks
 * are recomputed and ones that have changed are refilled.
 * The current implementation allows for different number of ld&hd chunks,
 * however this requires chunks that change from ld to hd (and vice versa)
 * to be completely regenerated.
 */
class InfiniteGrassWorld : public WorldGrass {
public:
  static constexpr auto HD_CHUNKS = InfiniteGrassWorldImpl::getHDChunksOffsets();
  static constexpr auto LD_CHUNKS = InfiniteGrassWorldImpl::getLDChunksOffsets();
  static constexpr float CAMERA_MOVEMENT_THRESHOLD = .2f; // the grass chunks will be regenerated when the camera moves by more than the threshold into another chunk
private:
  std::array<glm::ivec2, HD_CHUNKS.size()> m_hdGrassChunks;
  std::array<glm::ivec2, LD_CHUNKS.size()> m_ldGrassChunks;
  glm::ivec2                               m_currentCameraChunk;
  std::unique_ptr<GrassGenerator>          m_generator;
  unsigned int                             m_chunkSize;
  unsigned int                             m_bladesPerHDChunk;
  unsigned int                             m_bladesPerLDChunk;

public:
  InfiniteGrassWorld(std::unique_ptr<GrassGenerator> &&generator, glm::ivec2 creationGrassChunk, unsigned int chunkSize)
    : WorldGrass(Mathf::roundToNearestDivisor(GrassRenderSettings::MAX_BLADE_COUNT_PER_DRAWCALL, (unsigned int)m_hdGrassChunks.size()),
                 Mathf::roundToNearestDivisor(GrassRenderSettings::MAX_BLADE_COUNT_PER_DRAWCALL, (unsigned int)m_ldGrassChunks.size())),
    m_currentCameraChunk(creationGrassChunk), m_generator(std::move(generator)), m_chunkSize(chunkSize)
  {
    m_bladesPerHDChunk = GrassRenderSettings::MAX_BLADE_COUNT_PER_DRAWCALL / (unsigned int)m_hdGrassChunks.size();
    m_bladesPerLDChunk = GrassRenderSettings::MAX_BLADE_COUNT_PER_DRAWCALL / (unsigned int)m_ldGrassChunks.size();
  }

  void regenerate() override;
  void step(const Renderer::Camera &camera) override;

  inline glm::ivec2 positionToGrassChunk(const glm::vec3 &position)
  {
    return { glm::floor(position.x / m_chunkSize), glm::floor(position.z / m_chunkSize) };
  }

private:
  template<size_t CC>
  void repopulateGrassChunks(std::array<glm::ivec2, CC> &currentGrassChunks, const std::array<glm::ivec2, CC> &chunksOffsets, unsigned int instanceBuffer, unsigned int instanceCountPerBufferSlot);
};

/*
 * The simplest grass world implementation, it builds the grass buffers once
 * and never changes them again.
 * ld&hd chunks should probably not overlap, no checks are done by this class
 * to make sure that it is not the case, they would just be drawn on top of
 * one another.
 */
class FixedGrassChunks : public WorldGrass {
private:
  std::unique_ptr<GrassGenerator> m_generator;
  unsigned int                    m_chunkSize;
  std::vector<glm::ivec2>         m_hdChunks;
  std::vector<glm::ivec2>         m_ldChunks;
public:
  FixedGrassChunks(std::unique_ptr<GrassGenerator> &&generator,
                   unsigned int chunkSize,
                   const std::vector<glm::ivec2> &hdChunks,
                   const std::vector<glm::ivec2> &ldChunks)
    : WorldGrass(Mathf::roundToNearestDivisor(GrassRenderSettings::MAX_BLADE_COUNT_PER_DRAWCALL, (unsigned int)hdChunks.size()),
                 Mathf::roundToNearestDivisor(GrassRenderSettings::MAX_BLADE_COUNT_PER_DRAWCALL, (unsigned int)ldChunks.size())),
    m_generator(std::move(generator)), m_chunkSize(chunkSize), m_hdChunks(hdChunks), m_ldChunks(ldChunks)
  {
  }

  void regenerate() override;
  void step(const Renderer::Camera &camera) override {}
};

} // !namespace World

