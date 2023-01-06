#pragma once

#include "Terrain.h"

/**
* Terrain instances are generated from Heightmaps.
*
* Heightmaps used to generated a terrain of C chunks of size S must be 
* at least of size 3+C*S, the factor '3' is required because samples are
* taken out of bounds of the actual terrain chunks to generate normals.
*/
namespace Terrain {

// TODO the fact that this file is called MeshGenerator does not make mush sense
// it would make more sense to put everything in Terrain.h

Terrain generateTerrain(TerrainData terrainData, unsigned int chunkCountX, unsigned int chunkCountY, unsigned int chunkSize);
// Note: when calling this function the caller looses the ownership of heightMap!
Terrain generateTerrain(HeightMap *heightMap, unsigned int chunkCountX, unsigned int chunkCountY, unsigned int chunkSize);

}
