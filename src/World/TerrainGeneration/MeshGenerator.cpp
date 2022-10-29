#include "MeshGenerator.h"

using namespace Renderer;

namespace TerrainMeshGenerator {

Terrain::Terrain()
  : m_heightMap(nullptr),
  m_chunks()
{
}

Terrain::Terrain(HeightMap *heightmap)
  : m_heightMap(heightmap),
  m_chunks()
{
}

Terrain::Terrain(Terrain &&moved) noexcept
{
  m_heightMap = moved.m_heightMap;
  m_chunks = std::move(moved.m_chunks);
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


static Renderer::Mesh generateChunkMesh(const HeightMap &heightmap, glm::ivec2 chunkPosition, unsigned int chunkSize)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    for (int y = 0; y <= (int)chunkSize; y++) {
      for (int x = 0; x <= (int)chunkSize; x++) {
        int hx = chunkPosition.x * chunkSize + x + 1;
        int hy = chunkPosition.y * chunkSize + y + 1;
        Vertex& vertex = vertices.emplace_back();
        vertex.position = {
          x + chunkPosition.x * chunkSize,
          heightmap.getHeight(hx, hy),
          y + chunkPosition.y * chunkSize,
        };

        vertex.uv = { x, y };
        vertex.uv /= 10.f;

        vertex.normal = glm::normalize(glm::cross(
          glm::vec3{ 0.f, (heightmap.getHeight(hx, hy + 1) - heightmap.getHeight(hx, hy - 1)), 2.f},
          glm::vec3{ 2.f, (heightmap.getHeight(hx + 1, hy) - heightmap.getHeight(hx - 1, hy)), 0.f}
        ));

        vertex.color = {
          Mathf::rand(chunkPosition.x * 64.542f),
          Mathf::rand(chunkPosition.y * 12.523f),
          Mathf::rand(chunkPosition.x * 25.642f + chunkPosition.y * 53.2f),
        };

        vertex.texId = 1; // TODO remove? texID from vertices
        if (glm::dot(vertex.normal, glm::vec3{ 0.f,1.f,0.f }) > 0.3) vertex.texId = 0;
      }
    }

    for (int x = 0; x < (int)chunkSize; x++) {
      for (int y = 0; y < (int)chunkSize; y++) {
        unsigned int a1 = y * (chunkSize+1) + x;
        unsigned int a2 = y * (chunkSize+1) + x + 1;
        unsigned int a3 = (y + 1) * (chunkSize+1) + x;
        unsigned int a4 = (y + 1) * (chunkSize+1) + x + 1;
        indices.push_back(a1);
        indices.push_back(a3);
        indices.push_back(a2);
        indices.push_back(a2);
        indices.push_back(a3);
        indices.push_back(a4);
      }
    }

    Mesh mesh{ vertices, indices };
    return mesh;
}

static Chunk generateChunk(const HeightMap &heightmap, glm::ivec2 chunkPosition, unsigned int chunkSize)
{
  Renderer::Mesh mesh = generateChunkMesh(heightmap, chunkPosition, chunkSize);

  return Chunk(std::move(mesh));
}

// Convenient function
Terrain generateTerrain(TerrainData terrainData, unsigned int chunkCountX, unsigned int chunkCountY, unsigned int chunkSize)
{
  unsigned int noiseMapWidth = 3 + chunkSize * chunkCountX;
  unsigned int noiseMapHeight = 3 + chunkSize * chunkCountY;

  float *noiseMap = Noise::generateNoiseMap(noiseMapWidth,
                                            noiseMapHeight,
                                            terrainData.scale,
                                            terrainData.octaves,
                                            terrainData.persistence,
                                            terrainData.lacunarity,
                                            terrainData.seed);
  for(unsigned int i = 0; i < noiseMapWidth*noiseMapHeight; i++)
    noiseMap[i] *= terrainData.terrainHeight;

  HeightMap *heightMap = new ConcreteHeightMap(noiseMapWidth, noiseMapHeight, noiseMap);
  return generateTerrain(heightMap, chunkCountX, chunkCountY, chunkSize);
}

Terrain generateTerrain(HeightMap *heightMap, unsigned int chunkCountX, unsigned int chunkCountY, unsigned int chunkSize)
{
  assert(heightMap->getMapWidth() >= 3 + chunkCountX * chunkSize);
  assert(heightMap->getMapHeight() >= 3 + chunkCountY * chunkSize);

  Terrain terrain(heightMap);
    
  for (unsigned int x = 0; x < chunkCountX; x++) {
    for (unsigned int y = 0; y < chunkCountY; y++) {
      glm::ivec2 chunkPosition{ x, y };
      Chunk chunk = generateChunk(terrain.getHeightMap(), chunkPosition, chunkSize);
      terrain.getChunks().insert({ chunkPosition, std::move(chunk) });
    }
  }

  return terrain;
}
Terrain generateTerrain(const HeightMap& heightmap, unsigned int numberOfChunks, float depth) {

    Terrain terrain;
    terrain.heightMap = heightmap;


    unsigned int chunkSize = std::min(terrain.heightMap.getMapWidth() / numberOfChunks, terrain.heightMap.getMapHeight() / numberOfChunks);
    terrain.chunkSize = chunkSize;

    for (unsigned int i = 0; i < numberOfChunks * numberOfChunks; i++) {

        float r = ((float)rand() / (RAND_MAX));
        float g = ((float)rand() / (RAND_MAX));
        float b = ((float)rand() / (RAND_MAX));
        color_chunk = { r,g,b };
        glm::vec2 chunk_position = { i % numberOfChunks * chunkSize, i / numberOfChunks * chunkSize };

        HeightMapView hmv = HeightMapView(terrain.heightMap, chunk_position, glm::vec2((float)chunkSize));
        Chunk chunk = generateChunk(hmv, depth);
        terrain.chunksPosition.insert({ chunk_position, std::move(chunk) });
    }



    return terrain;
}

}