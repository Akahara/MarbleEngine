#include "Noise.h"

#include "PerlinNoise.hpp"
#include <stb/stb_image.h>

#include "../../Utils/Mathf.h"

namespace Noise {

float* generateNoiseMap(int mapWidth, int mapHeight, float scale, int octaves, float persistance, float initialFrequency, float lacunarity, int s) 
{
	assert(scale > 0); // TODO the scale parameter does not make much sense if the noise values are inverse-lerped back to 0..1
	assert(mapWidth > 0);
	assert(mapHeight > 0);
	float* noiseMap = new float[mapWidth * mapHeight]();

	const siv::PerlinNoise::seed_type seed = s;
	const siv::PerlinNoise perlin{ seed };

	float maxNoiseHeight = std::numeric_limits<float>::min();
	float minNoiseHeight = std::numeric_limits<float>::max();

	float amplitude = 1;
	float frequency = initialFrequency;
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
			noiseMap[y * mapWidth + x] = Mathf::inverseLerp(minNoiseHeight, maxNoiseHeight, noiseMap[y*mapWidth+x]);
		}
	}

	return noiseMap;
}

float *loadNoiseMapFromFile(const char *path, unsigned int *o_width, unsigned int *o_height)
{
  int mapWidth, mapHeight;
  unsigned char *buf = stbi_load(path, &mapWidth, &mapHeight, nullptr, 1);
  
  if (!buf) {
	std::cout << "Error: Failed to load noise texture '" << path << "'" << std::endl;
	std::cout << stbi_failure_reason() << std::endl;
	__debugbreak();
	stbi_image_free(buf);
	throw std::exception("Failed to load file");
  }

  float *noiseMap = new float[mapWidth * mapHeight];
  for (int x = 0; x < mapWidth; x++) {
	for (int y = 0; y < mapHeight; y++) {
	  noiseMap[x + y * mapWidth] = buf[x + y * mapWidth] / 255.f;
	}
  }

  stbi_image_free(buf);
  *o_width = (unsigned int)mapWidth;
  *o_height = (unsigned int)mapHeight;
  return noiseMap;
}

void rescaleNoiseMap(float *noiseMap, unsigned int mapWidth, unsigned int mapHeight, float currentMin, float currentMax, float newMin, float newMax)
{
  for (unsigned int x = 0; x < mapWidth; x++) {
	for (unsigned int y = 0; y < mapHeight; y++) {
	  noiseMap[x + y * mapWidth] = Mathf::mix(noiseMap[x + y * mapWidth], currentMin, currentMax, newMin, newMax);
	}
  }
}

void outlineNoiseMap(float *noiseMap, unsigned int mapWidth, unsigned int mapHeight, float outlineHeight, unsigned int outlineSize)
{
  for (unsigned int x = 0; x < mapWidth; x++) {
	for (unsigned int y = 0; y < outlineSize; y++) {
	  noiseMap[y * mapWidth + x] = outlineHeight;
	  noiseMap[(mapHeight-y-1) * mapWidth + x] = outlineHeight;
	}
  }
  for (unsigned int y = 0; y < mapHeight; y++) {
	for (unsigned int x = 0; x < outlineSize; x++) {
	  noiseMap[y * mapWidth + x] = outlineHeight;
	  noiseMap[y * mapWidth + (mapWidth-x-1)] = outlineHeight;
	}
  }
}

static void initializeErosionBrush(int mapSize, int erosionRadius, std::vector<std::vector<int>> &brushIndices, std::vector<std::vector<float>> &brushWeights)
{
  brushIndices = std::vector<std::vector<int>>(mapSize * mapSize);
  brushWeights = std::vector<std::vector<float>>(mapSize * mapSize);

  std::vector<int> xOffsets(erosionRadius * erosionRadius * 4);
  std::vector<int> yOffsets(erosionRadius * erosionRadius * 4);

  std::vector<float> weights(erosionRadius * erosionRadius * 4);

  for (int i = 0; i < brushIndices.size(); i++) {
	int centreX = i % mapSize;
	int centreY = i / mapSize;

	if (centreY < erosionRadius || centreY > mapSize - erosionRadius || centreX < erosionRadius || centreX > mapSize - erosionRadius)
	  continue;

	// find the cells covered by the brush, could be optimized
	float weightSum = 0;
	int lastBrushCellIndex = 0;
	for (int y = -erosionRadius; y <= erosionRadius; y++) {
	  for (int x = -erosionRadius; x <= erosionRadius; x++) {
		float sqrDst = (float)(x * x + y * y);

		if (sqrDst >= erosionRadius * erosionRadius)
		  continue;

		int coordX = centreX + x;
		int coordY = centreY + y;

		if (coordX < 0 || coordX >= mapSize || coordY < 0 || coordY >= mapSize)
		  continue;

		float weight = 1 - std::sqrt(sqrDst) / erosionRadius;
		weightSum += weight;
		weights[lastBrushCellIndex] = weight;
		xOffsets[lastBrushCellIndex] = x;
		yOffsets[lastBrushCellIndex] = y;
		lastBrushCellIndex++;
	  }
	}

	brushIndices[i].resize(lastBrushCellIndex);
	brushWeights[i].resize(lastBrushCellIndex);

	// actually fill the brush
	for (int j = 0; j < lastBrushCellIndex; j++) {
	  brushIndices[i][j] = (yOffsets[j] + centreY) * mapSize + xOffsets[j] + centreX;
	  brushWeights[i][j] = weights[j] / weightSum;
	}
  }
}

static glm::vec3 computeHeightGradientOfCell(float *map, unsigned int mapSize, float posX, float posY)
{
  int coordX = (int)posX;
  int coordY = (int)posY;

  float x = posX - coordX;
  float y = posY - coordY;

  // Get the gradient from the surroundings points
  // (0,0) is North west, (1,1) is south east

  float heightNW = map[coordX + coordY * mapSize];
  float heightNE = map[coordX + 1 + coordY * mapSize];
  float heightSW = map[coordX + (coordY + 1) * mapSize];
  float heightSE = map[coordX + 1 + (coordY + 1) * mapSize];

  // Compute the gradient (p.9)

  float gradientX = (heightNE - heightNW) * (1 - y) + (heightSE - heightSW) * y;
  float gradientY = (heightSW - heightNW) * (1 - x) + (heightSE - heightNE) * x;

  // Bilerp all the heights

  float height = heightNW * (1 - x) * (1 - y) + heightNE * x * (1 - y) + heightSW * (1 - x) * y + heightSE * x * y;

  return { gradientX, gradientY, height };
}

// TODO add the source of the erosion algorithm
void erode(float *map, unsigned int mapSize, const ErosionSettings &settings)
{
  std::vector<std::vector<int>> erosionBrushIndices;
  std::vector<std::vector<float>> erosionBrushWeights;
  initializeErosionBrush(mapSize, settings.erosionRadius, erosionBrushIndices, erosionBrushWeights);

  for (size_t interation = 0; interation < settings.dropletCount; interation++) {
	// Create water droplet at random point
	float posX = (float) (rand() % (mapSize - 2) + 1);
	float posY = (float) (rand() % (mapSize - 2) + 1);
	//posX = posY = mapSize / 2;
	float dirX = 0;
	float dirY = 0;
	float speed = settings.initialSpeed;
	float water = settings.initialWaterVolume;
	float sediment = 0;

	for (int lifetime = 0; lifetime < settings.maxDropletLifetime; lifetime++) {
	  // Compute droplet height + direction of flow with bilerp of surrounding height (p8?)
	  int nodeX = (int)posX;
	  int nodeY = (int)posY;

	  int dropletID = nodeY * mapSize + nodeX;

	  float cellOffsetX = posX - nodeX;
	  float cellOffsetY = posY - nodeY;

	  glm::vec3 heightAndGradient = computeHeightGradientOfCell(map, mapSize, posX, posY); // returns {g(X), g(Y), height}
	  // Update droplets pos and dir
	  dirX = (dirX * settings.inertia - heightAndGradient.x * (1 - settings.inertia));
	  dirY = (dirY * settings.inertia - heightAndGradient.y * (1 - settings.inertia));
	  if (dirX == 0 && dirY == 0) {
		dirX = 1;
		dirY = 0;
	  } else {
		glm::vec2 normalizedDirection = glm::normalize(glm::vec2(dirX, dirY));
		dirX = normalizedDirection.x;
		dirY = normalizedDirection.y;
	  }

	  posX += dirX;
	  posY += dirY;

	  // Break if the droplet is not valid (outside of the map or no mvmnt)
	  if ((dirX == 0 && dirY == 0) || posX < 0 || posX >= mapSize - 1 || posY < 0 || posY >= mapSize - 1) {
		break;
	  }

	  // find the droplets new height and compute deltaHeight
	  float newHeight = computeHeightGradientOfCell(map, mapSize, posX, posY).z;
	  float deltaHeight = newHeight - heightAndGradient.z;

	  // Calculate sediment capacity
	  float sedimentCapacity = std::max(-deltaHeight * speed * water * settings.sedimentCapacityFactor, settings.minSedimentCapacity); // apparently this should work too

	  // If carrying more sediment than capacity or droplet is going up a slope:
	  if (sediment > sedimentCapacity || deltaHeight > 0) {
		// deposit a fraction of the sediment to the surrounding nodes (with bilerp)
		float amountToDeposit = (deltaHeight > 0) ? std::min(deltaHeight, sediment) : (sediment - sedimentCapacity) * settings.depositSpeed;
		sediment -= amountToDeposit;

		map[dropletID] += amountToDeposit * (1 - cellOffsetX) * (1 - cellOffsetY);
		map[dropletID + 1] += amountToDeposit * cellOffsetX * (1 - cellOffsetY);
		map[dropletID + mapSize] += amountToDeposit * (1 - cellOffsetX) * cellOffsetY;
		map[dropletID + mapSize + 1] += amountToDeposit * cellOffsetX * cellOffsetY;
	  } else {
		// Erode a fraction of the droplets remaining capacity from the ground
		// dont erode more than deltaHeight

		float amountToErode = std::min((sedimentCapacity - sediment) * settings.erodeSpeed, -deltaHeight);

		for (int brushPointIndex = 0; brushPointIndex < erosionBrushIndices[dropletID].size(); brushPointIndex++) {
		  int nodeIndex = erosionBrushIndices[dropletID][brushPointIndex];
		  float weighedErodeAmount = amountToErode * erosionBrushWeights[dropletID][brushPointIndex];
		  float deltaSediment = (map[nodeIndex] < weighedErodeAmount) ? map[nodeIndex] : weighedErodeAmount;
		  map[nodeIndex] -= deltaSediment;
		  sediment += deltaSediment;
		}
	  }

	  // update dropplets speed based on deltaheight
	  // evaporate a fraction of the water of the droplet
	  speed = std::sqrt(std::max(0.f, speed * speed + deltaHeight * settings.gravity));
	  water *= (1 - settings.evaporateSpeed);
	}
  }
}

}