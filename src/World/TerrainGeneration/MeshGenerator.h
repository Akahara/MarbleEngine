#pragma once

#include <iostream>
#include <unordered_map>
#include <map>

#include <glm/glm.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/hash.hpp>

#include "../../abstraction/Mesh.h"
#include "../../Utils/Mathf.h"
#include "HeightMap.h"
#include "Noise.h"

namespace TerrainMeshGenerator {

    //============================================================================//

    // These values are kinda magical and good looking
    struct TerrainData {
<<<<<<< HEAD
      float scale = 27.6f;
      float terrainHeight = 20.f;
      int   octaves = 4;          // Number of times we add a new frequency to the heightmap
      float persistence = 0.3f;   // How impactfull an octave is relative to the previous one
      float lacunarity = 3.18f;   // How scaled an octave is relative to the previous one
      int   seed = 5;
=======
        unsigned int  width = 64;        // < Could be a single field as the terrain is squared
        unsigned int  height = 64;
        float         scale = 27.6f;
        float         terrainHeight = 20.f;
        int           octaves = 4;          // < Number of times we add a new frequency to the heightmap
        float         persistence = 0.3f;       // < How impactfull 
        float         lacunarity = 3.18f;
        int           seed = 5;

        float* noiseMap = Noise::generateNoiseMap(width, height, scale, octaves, persistence, lacunarity, seed);
>>>>>>> feat : end of the algorithm, doesnt work ! too bad !
    };

    //============================================================================//

    class Chunk {
    private:
      Renderer::Mesh m_mesh;

    public:
      Chunk() : m_mesh() {}
      Chunk(Renderer::Mesh &&mesh) : m_mesh(std::move(mesh)) {}

      Chunk(const Chunk&) = delete;
      Chunk(Chunk &&moved) noexcept;
      Chunk &operator=(const Chunk &) = delete;
      Chunk &operator=(Chunk &&moved) noexcept;

      const Renderer::Mesh &getMesh() const { return m_mesh; }
    };

    //============================================================================//

<<<<<<< HEAD
    class Terrain {
    private:
      HeightMap   *m_heightMap;
      std::unordered_map<glm::ivec2, Chunk> m_chunks;
    public:
      Terrain();
      Terrain(HeightMap *heightmap);
      Terrain(const Terrain&) = delete;
      Terrain &operator=(const Terrain &) = delete;
      Terrain(Terrain &&moved) noexcept;
      Terrain &operator=(Terrain &&moved) noexcept;
      ~Terrain();

      float getHeight(int x, int y) const { return m_heightMap->getHeight(x, y); }
      const HeightMap &getHeightMap() const { return *m_heightMap; }
      const std::unordered_map<glm::ivec2, Chunk> &getChunks() const { return m_chunks; }
      std::unordered_map<glm::ivec2, Chunk> &getChunks() { return m_chunks; }
=======
    struct Terrain {

        Terrain() : chunkSize(0), heightMap(), chunksPosition() {}
        Terrain(const Terrain&) = delete;
        Terrain(Terrain&& moved) noexcept {

            chunkSize = moved.chunkSize;
            heightMap = std::move(moved.heightMap);
            chunksPosition = std::move(moved.chunksPosition);
        }

        Terrain& operator=(const Terrain&) = delete;
        Terrain& operator=(Terrain&& moved) noexcept
        {
            this->~Terrain();
            new (this)Terrain(std::move(moved));
            return *this;
        }

        float getHeight(int x, int y) { return heightMap.getHeight(x, y); }




        //---------------------------/



        unsigned int chunkSize;
        unsigned int numberOfChunks;
        HeightMap heightMap;
        std::unordered_map<glm::vec2, Chunk> chunksPosition;


>>>>>>> feat : end of the algorithm, doesnt work ! too bad !
    };

    //============================================================================//

<<<<<<< HEAD
    Terrain generateTerrain(TerrainData terrainData, unsigned int chunkCountX, unsigned int chunkCountY, unsigned int chunkSize);
    // Note: when calling this function the caller looses the ownership of heightMap!
    Terrain generateTerrain(HeightMap *heightMap, unsigned int chunkCountX, unsigned int chunkCountY, unsigned int chunkSize);
=======
    Chunk generateChunk(const HeightMapView& heightmap, float depth); 

    Renderer::Mesh generateMesh(const HeightMapView& heightmap, float depth);

    Terrain generateTerrain(TerrainData terrainData, unsigned int numberOfChunks);

    Terrain generateTerrain(float* noiseMap, unsigned int w, unsigned int h, unsigned int numberOfChunks, float depth); 
    Terrain generateTerrain(const HeightMap& heightmap, unsigned int numberOfChunks, float depth);
>>>>>>> feat : end of the algorithm, doesnt work ! too bad !
}
