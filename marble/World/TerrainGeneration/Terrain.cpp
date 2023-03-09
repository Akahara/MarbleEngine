#include "Terrain.h"

#include "../../abstraction/UnifiedRenderer.h"
#include "../../Utils/Mathf.h"
#include "Noise.h"

namespace Renderer {

static IndexBufferObject generateChunkIBO()
{
  constexpr auto CHUNK_SIZE = TerrainMesh::CHUNK_SIZE;
  constexpr int indiceCount = CHUNK_SIZE * CHUNK_SIZE * 6;
  unsigned int *indices = new unsigned int[indiceCount];
  //std::array<unsigned int, CHUNK_SIZE*CHUNK_SIZE*6> indices{}; // does not need to be initialized but the warning wouldn't go away
  size_t i = 0;
  for (int x = 0; x < CHUNK_SIZE; x++) {
    for (int y = 0; y < CHUNK_SIZE; y++) {
      unsigned int a1 = y * (CHUNK_SIZE + 1) + x;
      unsigned int a2 = y * (CHUNK_SIZE + 1) + x + 1;
      unsigned int a3 = (y + 1) * (CHUNK_SIZE + 1) + x;
      unsigned int a4 = (y + 1) * (CHUNK_SIZE + 1) + x + 1;
      indices[i++] = a1;
      indices[i++] = a3;
      indices[i++] = a2;
      indices[i++] = a2;
      indices[i++] = a3;
      indices[i++] = a4;
    }
  }
  assert(i == indiceCount);
  IndexBufferObject ibo(indices, indiceCount);
  delete[] indices;
  return ibo;
}

TerrainMesh::TerrainMesh(const std::shared_ptr<Material> &material)
  : m_material(material), m_ibo(generateChunkIBO())
{
}

} // !namespace World