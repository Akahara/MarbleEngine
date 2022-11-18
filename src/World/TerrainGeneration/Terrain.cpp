#include "Terrain.h"

namespace Terrain {

Terrain::Terrain()
  : m_heightMap(nullptr),
  m_chunks()
{
}

Terrain::Terrain(HeightMap *heightmap, unsigned int chunkSize)
  : m_heightMap(heightmap),
  m_chunks(),
  m_chunkSize(chunkSize)
{
}

Terrain::Terrain(Terrain &&moved) noexcept
{
  m_heightMap = moved.m_heightMap;
  m_chunks = std::move(moved.m_chunks);
  m_chunkSize = moved.m_chunkSize;
  moved.m_heightMap = nullptr;
}

Terrain &Terrain::operator=(Terrain &&moved) noexcept
{
  this->~Terrain();
  new (this)Terrain(std::move(moved));
  return *this;
}

Terrain::~Terrain()
{
  delete m_heightMap;
}


Chunk::Chunk(Chunk &&moved) noexcept
{
  m_mesh = std::move(moved.m_mesh);
}

Chunk &Chunk::operator=(Chunk &&moved) noexcept
{
  this->~Chunk();
  new (this)Chunk(std::move(moved));
  return *this;
}

} // !namespace Terrain