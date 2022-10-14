#pragma once

#include <glm/glm.hpp>

#include "HeightMap.h"
#include "../../abstraction/Mesh.h"
#include "../../Utils/Mathf.h"
#include <glm/geometric.hpp>
#include <iostream>
#include <glm/gtx/hash.hpp>
#include <unordered_map>
#include "Noise.h"
#include "HeightMap.h"

namespace TerrainMeshGenerator {

    struct Chunk {

        Renderer::Mesh mesh;

        Chunk() : mesh() {}

        Chunk(const Chunk&) = delete;
        Chunk(Chunk&& moved) {

            mesh = std::move(moved.mesh);
        }

        Chunk& operator=(const Chunk&) = delete;
        Chunk& operator=(Chunk&& moved)
        {
            this->~Chunk();
            new (this)Chunk(std::move(moved));
            return *this;
        }


    };


    struct Terrain {

        Terrain() : chunkSize(0), heightMap(), chunksPosition() {}
        Terrain(const Terrain&) = delete;
        Terrain(Terrain&& moved) {

            chunkSize = moved.chunkSize;
            heightMap = std::move(moved.heightMap);
            chunksPosition = std::move(moved.chunksPosition);
        }

        Terrain& operator=(const Terrain&) = delete;
        Terrain& operator=(Terrain&& moved)
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

Renderer::Mesh generateMesh(const HeightMap &heightmap);

Renderer::Mesh generateMesh(const HeightMapView& heightmap);


Terrain generateTerrain(float* noiseMap, float w, float h, unsigned int chunkSize); 
}
