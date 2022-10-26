#pragma once

#include <glm/glm.hpp>
// https://www.firespark.de/resources/downloads/implementation%20of%20a%20methode%20for%20hydraulic%20erosion.pdf

class Erosion
{
private:

	/*
	
"Every drop stores the following information: Its position pos on the grid as a two
dimensional float vector, its current flowing direction dir as a two dimensional normalized float vector, its speed of motion vel as float value, the amount of water it
contains and the sediment  it carries both as float values."

	*/
	struct Droplet {

	private:
		glm::vec2 direction; // has to be normalized, call setDirection

	public:
		glm::vec2 position;
		float speed;
		float water;
		float sediment;

		void setDirection(const glm::vec2& dir) {

			assert(glm::length(dir) == 1);
			direction = dir;

		}

	};


public:

	Erosion() = delete;


	static void Erode(float* heightmap, int mapSize) {

		// Create water droplet at random point

		for (int lifetime = 0; lifetime < 30; lifetime++) {

			// Compute droplet height + direction of flow with bilerp of surrounding height (p8?)

			// Update droplets pos

			// find the droplets new height and compute deltaHeight

			// Calculate sediment capacity

			// If carrying more sediment than capacity or droplet is going up a slope:
					// deposit a fraction of the sediment to the surrounding nodes (with bilerp)

			// Else 
				// Erode a fraction of the droplets remaining capacity from the ground
				// dont erode more than deltaHeight

			// update dropplets speed based on deltaheight
			// evaporate a fraction of the water of the droplet


		}
	}











};

