#pragma once

#include <array>
#include <vector>

/**
* Noise can be generated manually or by using a function in this namespace.
* They can then be used to generate heightmaps.
*/
namespace Noise {

/* Standard perlin noise */
float *generateNoiseMap(int mapWidth, int mapHeight, float scale, int octaves, float persistance, float initialFrequency, float lacunarity, int seed);
/* Load a heightmap from a black and white file, white values produce heights of 1 and black values heights of 0 */
float *loadNoiseMapFromFile(const char *path, unsigned int *o_width, unsigned int *o_height);
/* Rescales a noisemap by applying a linear function to each of its values */
void rescaleNoiseMap(float *noiseMap, unsigned int mapWidth, unsigned int mapHeight, float currentMin, float currentMax, float newMin, float newMax);
/* Outline a noisemap by seting its edge values to the specified height, this can be used to produce walls or steep edges */
void outlineNoiseMap(float *noiseMap, unsigned int mapWidth, unsigned int mapHeight, float outlineHeight, unsigned int outlineSize);

// These values are kinda magical and good looking
struct TerrainData {
  float scale = 27.6f;          // the higher the scale, the flatter the terrain will apear to be
  float terrainHeight = 20.f;
  int   octaves = 4;            // Number of times we add a new frequency to the heightmap
  float persistence = 0.3f;     // How impactfull an octave is relative to the previous one
  float initialFrequency = 1.f; // The impactfullness of the first octave
  float lacunarity = 3.18f;     // How scaled an octave is relative to the previous one
  int   seed = 5;
};

struct ErosionSettings {
  int erosionRadius = 6;
  float inertia = .05f;             // At zero, water will instantly change direction to flow downhill. At 1, water will never change direction. 
  float sedimentCapacityFactor = 4; // Multiplier for how much sediment a droplet can carry
  float minSedimentCapacity = .01f; // Used to prevent carry capacity getting too close to zero on flatter terrain
  float erodeSpeed = .3f;           // speed at which droplets can take sediments
  float depositSpeed = .3f;         // speed at which droplets can release sediments
  float evaporateSpeed = .01f;      // fraction of the water lost after each step
  float gravity = 4;                // downward droplet speed
  float initialWaterVolume = 1;
  float initialSpeed = 1;
  int maxDropletLifetime = 30;      // number of erosion steps
  std::size_t dropletCount = 100'000;
};

/**
* Applies a standard erosion algorithm to an existing *square* noise map.
* 
* This algortihm is quite slow, prefer running it in release mode.
*
* Original algorithm: Implementation of a method for hydraulic erosion, Hans Theobald Beyer
* 
* FUTURE The erosion algorithm could (and should) be ran on the gpu
*/
void erode(float *noiseMap, unsigned int mapSize, const ErosionSettings &settings);

};

