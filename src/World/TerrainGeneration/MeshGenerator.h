#pragma once

#include <glm/glm.hpp>

#include "HeightMap.h"
#include "../../abstraction/Mesh.h"
#include "../../Utils/Mathf.h"
#include <glm/geometric.hpp>
#include <iostream>
#include <glm/gtx/hash.hpp>
#include <unordered_map>
#include <map>
#include "Noise.h"
#include "HeightMap.h"

namespace TerrainMeshGenerator {

    //============================================================================//

    // These values are kinda magical and good looking
    struct TerrainData {
        unsigned int  width = 200;        // < Could be a single field as the terrain is squared
        unsigned int  height = 200;
        float         scale = 27.6f;
        float         terrainHeight = 20.f;
        int           octaves = 4;          // < Number of times we add a new frequency to the heightmap
        float         persistence = 0.3f;       // < How impactfull 
        float         lacunarity = 3.18f;
        int           seed = 5;

        float* noiseMap = Noise::generateNoiseMap(width, height, scale, octaves, persistence, lacunarity, seed);
    };

    //============================================================================//

    struct Chunk {

        Renderer::Mesh mesh;

        Chunk() : mesh() {}

        Chunk(const Chunk&) = delete;
        Chunk(Chunk&& moved) noexcept {

            mesh = std::move(moved.mesh);
        }

        Chunk& operator=(const Chunk&) = delete;
        Chunk& operator=(Chunk&& moved) noexcept
        {
            this->~Chunk();
            new (this)Chunk(std::move(moved));
            return *this;
        }


    };

    //============================================================================//

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

        //---------------------------/

        unsigned int chunkSize;
        unsigned int numberOfChunks;
        HeightMap heightMap;
        std::unordered_map<glm::vec2, Chunk> chunksPosition;


    };

    //============================================================================//

    Chunk generateChunk(const HeightMapView& heightmap, float depth); 

    Renderer::Mesh generateMesh(const HeightMapView& heightmap, float depth);

    Terrain generateTerrain(TerrainData terrainData, unsigned int numberOfChunks);

    Terrain generateTerrain(float* noiseMap, unsigned int w, unsigned int h, unsigned int numberOfChunks, float depth); 
}
