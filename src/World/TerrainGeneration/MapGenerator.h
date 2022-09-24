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

public:

	MapGenerator(int w, int h, int scale, int o, float p, float l) 
		: mapWidth(w), mapHeight(h), noiseScale(scale), octaves(o), persistance(p), lacunarity(l) {}

	unsigned int GenerateMap() {

		float* noiseMap = Noise::GenerateNoiseMap(mapWidth, mapHeight, noiseScale, octaves, persistance, lacunarity);
		
		// Texture stuff

		unsigned int texture;
		(glGenTextures(1, &texture));
		(glBindTexture(GL_TEXTURE_2D, texture));

		(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,  GL_LINEAR));
		(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

		if (noiseMap)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mapWidth, mapHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, noiseMap);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		//testwriteTextureToFile(texture, "res.png");
		std::cout << "End of generation" << std::endl;
		delete[] noiseMap;
		return texture;
	}





};

