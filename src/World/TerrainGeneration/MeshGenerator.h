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
        HeightMap heightMap;
        std::unordered_map<glm::vec2, Chunk> chunksPosition;


    };

Renderer::Mesh generateMesh(const HeightMap &heightmap, float depth);

Renderer::Mesh generateMesh(const HeightMapView& heightmap, float depth);


Terrain generateTerrain(float* noiseMap, unsigned int w, unsigned int h, unsigned int numberOfChunks, float depth); 
}
