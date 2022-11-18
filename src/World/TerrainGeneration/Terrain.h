#pragma once

#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include "HeightMap.h"
#include "../../abstraction/Mesh.h"
#include "../../abstraction/Camera.h"

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
  const HeightMap &getHeightMap() const { return *m_heightMap; }
  const std::unordered_map<glm::ivec2, Chunk> &getChunks() const { return m_chunks; }
  std::unordered_map<glm::ivec2, Chunk> &getChunks() { return m_chunks; }
};

// These values are kinda magical and good looking
struct TerrainData {
  float scale = 27.6f;
  float terrainHeight = 20.f;
  int   octaves = 4;          // Number of times we add a new frequency to the heightmap
  float persistence = 0.3f;   // How impactfull an octave is relative to the previous one
  float lacunarity = 3.18f;   // How scaled an octave is relative to the previous one
  int   seed = 5;
};

}