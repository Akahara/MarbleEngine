#pragma once

#include <iostream>

#include "Noise.h"
#include "../../abstraction/Texture.h"


class MapGenerator
{
private:
	int mapWidth, mapHeight;
	float noiseScale;

	int octaves;
	float persistance, lacunarity;

	int seed;

	unsigned int m_renderTexture;

public:

	MapGenerator(int w, int h, float scale, int o, float p, float l, int s)
		: mapWidth(w), mapHeight(h), noiseScale(scale), octaves(o), persistance(p), lacunarity(l), seed(s), m_renderTexture(0) { }

	unsigned int GenerateTextureMap() {

		float* noiseMap = Noise::GenerateNoiseMap(mapWidth, mapHeight, noiseScale, octaves, persistance, lacunarity, seed);
		
		// Texture stuff

		(glGenTextures(1, &m_renderTexture));
		(glBindTexture(GL_TEXTURE_2D, m_renderTexture));

		(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

		if (noiseMap)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, mapWidth, mapHeight, 0, GL_RED, GL_FLOAT, noiseMap);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		//testwriteTextureToFile(texture, "res.png");
		std::cout << "End of generation" << std::endl;
		delete[] noiseMap;
		return m_renderTexture;
	}

};

