#pragma once

#include "Terrain.h"

namespace Terrain {

// TODO the fact that this file is called MeshGenerator does not make mush sense
// it would make more sense to put everything in Terrain.h

Terrain generateTerrain(TerrainData terrainData, unsigned int chunkCountX, unsigned int chunkCountY, unsigned int chunkSize);
// Note: when calling this function the caller looses the ownership of heightMap!
Terrain generateTerrain(HeightMap *heightMap, unsigned int chunkCountX, unsigned int chunkCountY, unsigned int chunkSize);

}
