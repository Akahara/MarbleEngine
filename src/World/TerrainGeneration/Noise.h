#pragma once

#include <array>
#include <vector>

namespace Noise {

float *generateNoiseMap(int mapWidth, int mapHeight, float scale, int octaves, float persistance, float initialFrequency, float lacunarity, int seed);
float *loadNoiseMapFromFile(const char *path, unsigned int *o_width, unsigned int *o_height);
void rescaleNoiseMap(float *noiseMap, unsigned int mapWidth, unsigned int mapHeight, float currentMin, float currentMax, float newMin, float newMax);

// TODO comment individual erosion settings
// TODO move the TerrainSettings struct in noise.h
// TerrainSettings is responsible for the heightmap generation, not the terrain generation itself
struct ErosionSettings {
  int erosionRadius = 6;
  float inertia = .05f;             // At zero, water will instantly change direction to flow downhill. At 1, water will never change direction. 
  float sedimentCapacityFactor = 4; // Multiplier for how much sediment a droplet can carry
  float minSedimentCapacity = .01f; // Used to prevent carry capacity getting too close to zero on flatter terrain
  float erodeSpeed = .3f;
  float depositSpeed = .3f;
  float evaporateSpeed = .01f;
  float gravity = 4;
  float initialWaterVolume = 1;
  float initialSpeed = 1;
  int maxDropletLifetime = 30;
  size_t dropletCount = 100'000 * 5;
};

void erode(float *noiseMap, unsigned int mapSize, const ErosionSettings &settings);

};

