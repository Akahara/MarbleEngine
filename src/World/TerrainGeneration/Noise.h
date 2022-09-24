#pragma once

#include "PerlinNoise.hpp"

#include <array>
#include <vector>



namespace Noise
{

	 float* GenerateNoiseMap(int mapWidth, int mapHeight, float scale, int octaves, float persistance, float lacunarity);


};

