#define GL_ENABLE_EXPERIMENTAL
#include "MeshGenerator.h"



using namespace Renderer;

namespace TerrainMeshGenerator {

  

Chunk generateChunk(const HeightMapView& heightmap) {


    Chunk chunk;
    chunk.mesh = generateMesh(heightmap);
    return chunk;

}

// TODO : concreteMap
Mesh generateMesh(const HeightMap &heightmap )
{
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;

  for (int y = 0; y < (int)heightmap.getMapHeight(); y++) {
    for (int x = 0; x < (int)heightmap.getMapWidth(); x++) {
      Vertex &vertex = vertices.emplace_back();
      vertex.position = { x , heightmap.getHeight(x, y) * 100, y };
      vertex.uv = { (float)x / heightmap.getMapWidth(), (float)y / heightmap.getMapHeight() };
      vertex.uv *= 10;

      glm::vec3 A, B;      
      A = { 0 , heightmap.getHeight(x + 1, y) - heightmap.getHeight(x, y) , 1.f / heightmap.getMapHeight()};
      B = { 1.f/ heightmap.getMapWidth()  , heightmap.getHeight(x + 1, y + 1) - heightmap.getHeight(x, y), 0};
      glm::vec3 N = glm::cross(A, B);

      vertex.normal = glm::normalize(N);
    }
  }

  for (int x = 0; x < (int)heightmap.getMapWidth() - 1; x++) {
    for (int y = 0; y < (int)heightmap.getMapHeight()-1; y++) {
      unsigned int a1 = y * heightmap.getMapWidth() + x;
      unsigned int a2 = y * heightmap.getMapWidth() + x + 1;
      unsigned int a3 = (y + 1) * heightmap.getMapWidth() + x;
      unsigned int a4 = (y + 1) * heightmap.getMapWidth() + x + 1;
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

// TODO ; enelver ca
Mesh generateMesh(const HeightMapView& heightmap )
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    for (int y = 0; y < (int)heightmap.getMapHeight(); y++) {
        for (int x = 0; x < (int)heightmap.getMapWidth(); x++) {
            Vertex& vertex = vertices.emplace_back();
            vertex.position = { x , heightmap.getHeight(x, y) * 0, y };
            vertex.uv = { (float)x / (heightmap.getMapWidth()*8), (float)y / heightmap.getMapHeight()/8 };
            vertex.uv *= 10;


            glm::vec3 A, B;
            A = { 0 , heightmap.getHeight(x + 1, y) - heightmap.getHeight(x, y) , 1.f / heightmap.getMapHeight() };
            B = { 1.f / heightmap.getMapWidth()  , heightmap.getHeight(x + 1, y + 1) - heightmap.getHeight(x, y), 0 };
            glm::vec3 N = glm::cross(A, B);

            vertex.normal = glm::normalize(N);
        }
    }

    for (int x = 0; x < (int)heightmap.getMapWidth() - 1; x++) {
        for (int y = 0; y < (int)heightmap.getMapHeight() - 1; y++) {
            unsigned int a1 = y * heightmap.getMapWidth() + x;
            unsigned int a2 = y * heightmap.getMapWidth() + x + 1;
            unsigned int a3 = (y + 1) * heightmap.getMapWidth() + x;
            unsigned int a4 = (y + 1) * heightmap.getMapWidth() + x + 1;
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


Terrain generateTerrain(float* noiseMap, float w, float h, unsigned int chunkSize) {

    Terrain terrain;
    terrain.heightMap.setHeights(w, h, noiseMap);
    
    terrain.chunkSize = chunkSize;

    int numberOfChunksSide = std::min(terrain.heightMap.getMapWidth() / terrain.chunkSize, terrain.heightMap.getMapHeight() / terrain.chunkSize);
    int numberOfChunks = numberOfChunksSide * numberOfChunksSide;

    for (unsigned int i = 0; i < numberOfChunks; i++) { // todo calculer le nombre de chunks

        glm::vec2 chunk_position = { i % numberOfChunksSide , i / numberOfChunksSide  };

        HeightMapView hmv = HeightMapView(terrain.heightMap, chunk_position, glm::vec2(chunkSize));
        Chunk chunk = generateChunk(hmv);
        terrain.chunksPosition.insert({ chunk_position, std::move(chunk) }); // todo calculer la position
    }



    return terrain;
}

}