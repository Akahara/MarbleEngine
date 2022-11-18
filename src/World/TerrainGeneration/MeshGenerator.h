#pragma once

#include "Terrain.h"

namespace Terrain {

Terrain generateTerrain(TerrainData terrainData, unsigned int chunkCountX, unsigned int chunkCountY, unsigned int chunkSize);
// Note: when calling this function the caller looses the ownership of heightMap!
Terrain generateTerrain(HeightMap *heightMap, unsigned int chunkCountX, unsigned int chunkCountY, unsigned int chunkSize);

}
