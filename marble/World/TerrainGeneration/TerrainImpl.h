#pragma once

#include <glm/glm.hpp>

#include "HeightMap.h"
#include "../../abstraction/Mesh.h"
#include "../../Utils/Mathf.h"

namespace Renderer {

template<Heightmap Heightmap>
TerrainMesh::Chunk TerrainMesh::generateChunk(const Heightmap &heightmap, glm::ivec2 chunkPosition)
{
  constexpr int vertexCount = (CHUNK_SIZE + 1) * (CHUNK_SIZE + 1);
  std::array<BaseVertex, vertexCount> vertices;

  size_t i = 0;
  for (int y = 0; y <= (int)CHUNK_SIZE; y++) {
    for (int x = 0; x <= (int)CHUNK_SIZE; x++) {
      float wx = (float)chunkPosition.x * CHUNK_SIZE + x + 1;
      float wy = (float)chunkPosition.y * CHUNK_SIZE + y + 1;
      BaseVertex &vertex = vertices[i++];
      vertex.position = { wx, heightmap(wx, wy), wy };

      vertex.uv = { x, y };
      vertex.uv /= 10.f;

      vertex.normal = glm::normalize(glm::cross(
        glm::vec3{ 0.f, (heightmap(wx, wy + 1) - heightmap(wx, wy - 1)), 2.f },
        glm::vec3{ 2.f, (heightmap(wx + 1, wy) - heightmap(wx - 1, wy)), 0.f }
      ));

      vertex.color = {
        Mathf::rand(chunkPosition.x * 64.542f),
        Mathf::rand(chunkPosition.y * 12.523f),
        Mathf::rand(chunkPosition.x * 25.642f + chunkPosition.y * 53.2f),
      };

      vertex.texId = 0;
    }
  }

  assert(i == vertexCount);

  glm::vec3 minAABB(std::numeric_limits<float>::max()), maxAABB(std::numeric_limits<float>::min());
  for (const auto &vertex : vertices) {
    minAABB = glm::min(minAABB, vertex.position);
    maxAABB = glm::max(maxAABB, vertex.position);
  }

  VertexBufferObject vbo{ vertices.data(), vertices.size()*sizeof(BaseVertex) };
  VertexArray vao;
  vao.addBuffer(vbo, BaseVertex::getVertexBufferLayout(), m_ibo);

  return Chunk{
    std::move(vao),
    std::move(vbo),
    chunkPosition,
    AABB::make_aabb(minAABB, maxAABB),
  };
}

template<Heightmap Heightmap>
void TerrainMesh::rebuildMesh(const Heightmap &heightmap, TerrainRegion region)
{
  // remove chunks that are no longer in the region
  m_chunks.erase(std::remove_if(m_chunks.begin(), m_chunks.end(), [region](const Chunk &chunk) {
    return
      chunk.position.y > region.maxY ||
      chunk.position.x > region.maxX ||
      chunk.position.y + 1 < region.minY ||
      chunk.position.x + 1 < region.minX;
  }), m_chunks.end());
  // create the new ones
  for (int chunkX = (int)(region.minX / CHUNK_SIZE); chunkX < glm::ceil(region.maxX / CHUNK_SIZE); chunkX++) {
    for (int chunkY = (int)(region.minY / CHUNK_SIZE); chunkY < glm::ceil(region.maxY / CHUNK_SIZE); chunkY++) {
      glm::ivec2 chunkPosition{ chunkX, chunkY };
      m_chunks.push_back(generateChunk(heightmap, chunkPosition));
    }
  }
}

}