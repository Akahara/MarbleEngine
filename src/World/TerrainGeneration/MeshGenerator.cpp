#define GL_ENABLE_EXPERIMENTAL
#include "MeshGenerator.h"



using namespace Renderer;
static glm::vec3 color_chunk = { 0.f, 0.f, 0.f };

namespace TerrainMeshGenerator {

  

Chunk generateChunk(const HeightMapView& heightmap, float depth) {


    Chunk chunk;
    chunk.mesh = generateMesh(heightmap, depth);
    return chunk;

}

Mesh generateMesh(const HeightMapView& heightmap, float depth)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    for (int y = 0; y < (int)heightmap.getMapHeight(); y++) {
        for (int x = 0; x < (int)heightmap.getMapWidth(); x++) {
            Vertex& vertex = vertices.emplace_back();
            vertex.position = { x , heightmap.getHeight(x, y) * depth, y };


            glm::vec2 originPoint = heightmap.getOriginPoint();
            vertex.uv = { originPoint.x + (float)x / (heightmap.getOriginMap().getMapWidth()), originPoint.y + (float)y / heightmap.getOriginMap().getMapHeight()};
            vertex.uv *= 10;


            glm::vec3 A, B;
            A = { 0 , heightmap.getHeight(x + 1, y) - heightmap.getHeight(x, y) , 1.f / heightmap.getOriginMap().getMapHeight() };
            B = { 1.f / heightmap.getOriginMap().getMapWidth()  , heightmap.getHeight(x + 1, y + 1) - heightmap.getHeight(x, y), 0};
            glm::vec3 N = glm::cross(A, B);

            vertex.normal = glm::normalize(N);
            vertex.color = color_chunk;
            vertex.texId = 1;
            if (glm::dot(vertex.normal, glm::vec3{ 0.f,1.f,0.f }) > 0.9) vertex.texId = 2;

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

Terrain generateTerrain(float* noiseMap, unsigned int w, unsigned int h, unsigned int numberOfChunks, float depth) {

    Terrain terrain;
    terrain.heightMap.setHeights(w, h, noiseMap);
    

    unsigned int chunkSize = std::min(terrain.heightMap.getMapWidth() / numberOfChunks, terrain.heightMap.getMapHeight() / numberOfChunks);
    terrain.chunkSize = chunkSize;

    for (unsigned int i = 0; i < numberOfChunks * numberOfChunks ; i++) {
        
        float r = ((float)rand() / (RAND_MAX));
        float g = ((float)rand() / (RAND_MAX));
        float b = ((float)rand() / (RAND_MAX));
        color_chunk = {r,g,b};
        glm::vec2 chunk_position = { i % numberOfChunks * chunkSize, i / numberOfChunks * chunkSize };

        HeightMapView hmv = HeightMapView(terrain.heightMap, chunk_position, glm::vec2((float)chunkSize));
        Chunk chunk = generateChunk(hmv, depth);
        terrain.chunksPosition.insert({ chunk_position, std::move(chunk) });
    }



    return terrain;
}

}