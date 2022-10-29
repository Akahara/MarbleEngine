#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>

#include "../HeightMap.h"

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
		float speed = 1;
		float water = 1;
		float sediment = 0;
		float height = 1;

		Droplet(const glm::vec2& pos) : position(pos)
		{

		}

		void setDirection(const glm::vec2& dir) {

			direction = glm::normalize(dir);

		}

		void updatePosition() {
			position += direction;
		}



	};

	//------------------------//





public:

	Erosion() = delete;


	static void Erode(HeightMap& heightmap, unsigned int nbDroplets=1) {

		// -- INITIALISATIONS

		unsigned int maxDropletLifetime = 30;
		float sedimentCapacityFactor = 4.f;
		float minSedimentCapacity = 0.01f;
		float depositSpeed = .3f;
		float gravity = 4;
		float erodeSpeed = .3f;
		float evaporateSpeed = .01f;

		// Create water droplet at random point
		
		for (int dropletID = 0; dropletID < nbDroplets; dropletID++) {

			Droplet droplet{

				{	rand() % heightmap.getMapWidth() ,
					rand() % heightmap.getMapHeight()
				}
			};

			//std::cout << heightmap.getHeight(droplet.position.x, droplet.position.y) << " (" << droplet.position.x << "," << droplet.position.y << ")";

			float dirX = 0;
			float dirY = 0;
			float inertia = 0.3;



			for (int maxDropletLifetime = 0; maxDropletLifetime < 30; maxDropletLifetime++) {

				// Compute droplet height + direction of flow with bilerp of surrounding height (p8?)

				droplet.height = heightmap.getHeight(droplet.position.x, droplet.position.y); // Multiply by the terrain hight ?

				glm::vec3 heightAndGradient = computeHeightGradientOfCell(heightmap, droplet.position.x, droplet.position.y); // returns {height, g(X), g(Y)}
				

				// Update droplets pos and dir
				/*
				float cellOffsetX = posX - nodeX;
				float cellOffsetY = posY - nodeY;
				*/

				dirX = (dirX * inertia - heightAndGradient.y * (1 - inertia));
				dirY = (dirY * inertia - heightAndGradient.z * (1 - inertia));

				glm::vec2 newDirection = glm::normalize( glm::vec2(dirX, dirY) );
				
				droplet.setDirection(newDirection);
				droplet.updatePosition();


				// Break if the droplet is not valid (outside of the map or no mvmnt)
				if ((dirX == 0 && dirY == 0) || droplet.position.x < 0 || droplet.position.x >=  heightmap.getMapWidth() - 1 || droplet.position.y < 0 || droplet.position.y >= heightmap.getMapHeight() - 1) {
					break;
				}

				// find the droplets new height and compute deltaHeight

				float deltaHeight = droplet.height - heightAndGradient.x;

				// Calculate sediment capacity

				float sedimentCapacity = std::max(-deltaHeight * droplet.speed * droplet.water * sedimentCapacityFactor, minSedimentCapacity); // apparently this should work too


				// If carrying more sediment than capacity or droplet is going up a slope:
				if (droplet.sediment > sedimentCapacity || deltaHeight > 0) {

						// deposit a fraction of the sediment to the surrounding nodes (with bilerp)
						
																		// DO NOT DROP MORE THAN DELTAHEIGHT
						float amountToDeposit = (deltaHeight > 0) ? std::min(deltaHeight, droplet.sediment) : (droplet.sediment - sedimentCapacity) * depositSpeed;
						droplet.sediment -= amountToDeposit;

						// Add the sediment to the four nodes of the current cell using bilinear interpolation
						// Deposition is not distributed over a radius (like erosion) so that it can fill small pits
						float val = amountToDeposit * (1 - droplet.position.x) * (1 - droplet.position.y);

						heightmap.addHeightAt(droplet.position.x, droplet.position.y, val);

						val = amountToDeposit * droplet.position.x * (1 - droplet.position.y);

						heightmap.addHeightAt(droplet.position.x, droplet.position.y, val);

						val = amountToDeposit * (1 - droplet.position.x) * droplet.position.y;

						heightmap.addHeightAt(droplet.position.x, droplet.position.y, val);

						val = amountToDeposit * droplet.position.x * droplet.position.y;

						heightmap.addHeightAt(droplet.position.x, droplet.position.y, val);



				}

				// Else $
				else {
					// Erode a fraction of the droplets remaining capacity from the ground
					// dont erode more than deltaHeight

					float amountToErode = std::min((sedimentCapacity - droplet.sediment) * erodeSpeed, -deltaHeight);

					// shitty brush thing, wtf does it mean 
					// TODO droplet radius 

					heightmap.addHeightAt(droplet.position.x, droplet.position.y, -amountToErode);
					droplet.sediment += amountToErode;

				}


				// update dropplets speed based on deltaheight
				// evaporate a fraction of the water of the droplet

				droplet.speed = std::sqrt(droplet.speed * droplet.speed + deltaHeight * gravity);
				droplet.water *= (1 - evaporateSpeed);


			}
			//std::cout << " : " << heightmap.getHeight(droplet.position.x, droplet.position.y) <<  " (" << droplet.position.x << "," << droplet.position.y << ")" << std::endl;
		}
	}

	static glm::vec3 computeHeightGradientOfCell(const HeightMap& heightmap, int x, int y) {

		// Get the gradient from the surroundings points
		// (0,0) is North west, (1,1) is south east

		float heightNW = heightmap.getHeight(x, y);
		float heightNE = heightmap.getHeight(x + 1, y);
		float heightSW = heightmap.getHeight(x, y + 1);;
		float heightSE = heightmap.getHeight(x + 1, y + 1);

		// Compute the gradient (p.9)

		float gradientX = (heightNE - heightNW) * (1 - y) + (heightSE - heightSW) * y;
		float gradientY = (heightSW - heightNW) * (1 - x) + (heightSE - heightNE) * x;

		// Apparently this computes the new height
		// Something like bilerp all the heights

		float height = heightNW * (1 - x) * (1 - y) + heightNE * x * (1 - y) + heightSW * (1 - x) * y + heightSE * x * y;

		return { height, gradientX, gradientY };
	}











};

