
#include "Noise.h"
#include <glm/gtc/noise.hpp>

static inline float inverseLerp(float xx, float yy, float value)
{
	return (value - xx) / (yy - xx);
}

namespace Noise {

	float* generateNoiseMap(int mapWidth, int mapHeight, float scale, int octaves, float persistance, float lacunarity, int s) 
	{
	    assert(scale > 0);
		float* noiseMap = new float[mapWidth * mapHeight]();

		const siv::PerlinNoise::seed_type seed = s;
		const siv::PerlinNoise perlin{ seed };

		float maxNoiseHeight = std::numeric_limits<float>::min();
		float minNoiseHeight = std::numeric_limits<float>::max();

		float amplitude = 1;
		float frequency = 1;
		for (int o = 0; o < octaves; o++) {
			for (int y = 0; y < mapHeight; y++) {
				for (int x = 0; x < mapWidth; x++) {
					float sampleX = x / scale * frequency;
					float sampleY = y / scale * frequency;
					double pValue = perlin.noise2D_01(sampleX, sampleY) * 2 - 1;

					noiseMap[y * mapWidth + x] += (float)pValue * amplitude;
				}
			}
			amplitude *= persistance; // persistance is [0;1]
			frequency *= lacunarity;
		}

		for (int y = 0; y < mapHeight; y++) {
			for (int x = 0; x < mapWidth; x++) {
				float noiseHeight = noiseMap[y * mapWidth + x];
				// Normalizing the values
				if (noiseHeight > maxNoiseHeight) maxNoiseHeight = noiseHeight;
				else if (noiseHeight < minNoiseHeight) minNoiseHeight = noiseHeight;
			}
		}

		for (int y = 0; y < mapHeight; y++) {
			for (int x = 0; x < mapWidth; x++) {
				noiseMap[y * mapWidth + x] = inverseLerp(minNoiseHeight, maxNoiseHeight, noiseMap[y*mapWidth+x]);
			}
		}

		return noiseMap;
	}


}