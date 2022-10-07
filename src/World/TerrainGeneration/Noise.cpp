
#include "Noise.h"
#include <glm/gtc/noise.hpp>

float InverseLerp(float xx, float yy, float value)
{
	return (value - xx) / (yy - xx);
}

namespace Noise {

	float* GenerateNoiseMap(int mapWidth, int mapHeight, float scale, int octaves, float persistance, float lacunarity, int s) {
		
		float* noiseMap = new float[mapWidth * mapHeight];

		const siv::PerlinNoise::seed_type seed = s;
		const siv::PerlinNoise perlin{ seed };


		if (scale <= 0) scale = 0.0001f;



		float maxNoiseHeight = -1000.f;
		float minNoiseHeight = 1000.f;

		for (int y = 0; y < mapHeight; y++) {

			

			for (int x = 0; x < mapWidth; x++) {

				float amplitude = 1;
				float frequency = 1;
				float noiseHeight = 0;

				for (int o = 0; o < octaves; o++) {


					float sampleX = x / scale * frequency;
					float sampleY = y / scale * frequency;

					double pValue = perlin.noise2D_01(sampleX, sampleY) * 2 - 1; 
					noiseHeight += (float)pValue * amplitude;

					amplitude *= persistance; // persistance is [0;1]
					frequency *= lacunarity;


					
				}

				// Normalizing the values
				if (noiseHeight > maxNoiseHeight) maxNoiseHeight = noiseHeight;
				else if (noiseHeight < minNoiseHeight) minNoiseHeight = noiseHeight;

				noiseMap[y * mapWidth + x] = noiseHeight;

			}
		}

		for (int y = 0; y < mapHeight; y++) {



			for (int x = 0; x < mapWidth; x++) {

				noiseMap[y * mapWidth + x] = InverseLerp(minNoiseHeight, maxNoiseHeight, noiseMap[y*mapWidth+x]);

			}
		}

		return noiseMap;


	}


}