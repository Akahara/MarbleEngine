#pragma once

#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include "Noise.h"
#include "HeightMap.h"
#include "../../abstraction/Mesh.h"
#include "../../abstraction/Camera.h"

/**
* Terrain are composed of chunks (not necessarily contiguous) and is defined
* by a height map. 
* 
* Each chunk is defined by a mesh that can be rendered but not modified, the
* chunk's bounding box (AABB) can be used not to render chunks that are not
* visible by a camera.
* 
* To get an heightmap see Noise.h and HeightMap.h
*/
namespace Terrain {

class Chunk {
private:
  Renderer::Mesh m_mesh;

public:
  Chunk() : m_mesh() {}
  Chunk(Renderer::Mesh &&mesh) : m_mesh(std::move(mesh)) {}

  Chunk(const Chunk &) = delete;
  Chunk(Chunk &&moved) noexcept;
  Chunk &operator=(const Chunk &) = delete;
  Chunk &operator=(Chunk &&moved) noexcept;

  const Renderer::Mesh &getMesh() const { return m_mesh; }
};

class Terrain {
private:
  HeightMap *m_heightMap;
  unsigned int m_chunkSize;
  std::unordered_map<glm::ivec2, Chunk> m_chunks;
public:
  Terrain();
  Terrain(HeightMap *heightmap, unsigned int chunkSize);
  Terrain(const Terrain &) = delete;
  Terrain &operator=(const Terrain &) = delete;
  Terrain(Terrain &&moved) noexcept;
  Terrain &operator=(Terrain &&moved) noexcept;
  ~Terrain();

  unsigned int getChunkSize() const { return m_chunkSize; }
  float getHeight(int x, int y) const { return m_heightMap->getHeight(x + 1, y + 1); } // beware! the heightmap has a padding of 1
  float getHeight(float x, float y) const { return m_heightMap->getHeightLerp(x + 1, y + 1); }
  bool isInSamplableRegion(float x, float y) const { return m_heightMap->isInBounds(x + 1, y + 1) && m_heightMap->isInBounds(x + 2, y + 2); }
  const HeightMap &getHeightMap() const { return *m_heightMap; }
  const std::unordered_map<glm::ivec2, Chunk> &getChunks() const { return m_chunks; }
  std::unordered_map<glm::ivec2, Chunk> &getChunks() { return m_chunks; }
};

/**
* Terrain instances are generated from Heightmaps.
*
* Heightmaps used to generated a terrain of C chunks of size S must be
* at least of size 3+C*S, the factor '3' is required because samples are
* taken out of bounds of the actual terrain chunks to generate normals.
*/
Terrain generateTerrain(Noise::TerrainData terrainData, unsigned int chunkCountX, unsigned int chunkCountY, unsigned int chunkSize);
// Note: when calling this function the caller looses the ownership of heightMap!
Terrain generateTerrain(HeightMap *heightMap, unsigned int chunkCountX, unsigned int chunkCountY, unsigned int chunkSize);

}