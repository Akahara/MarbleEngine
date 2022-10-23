#pragma once

#include "PerlinNoise.hpp"

#include <array>
#include <vector>



namespace Noise
{


	 float* generateNoiseMap(int mapWidth, int mapHeight, float scale
							, int octaves, float persistance, float lacunarity, int s );


};

