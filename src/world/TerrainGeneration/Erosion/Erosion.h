#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>

#include <vector>
#include <array>

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



	static void Erode(HeightMap& heightmap, unsigned int nbDroplets=100) {

		// -- INITIALISATIONS

		ConcreteHeightMap map = ConcreteHeightMap::normalize(heightmap); // heightmap is normalized i think

		int mapSize = std::max(map.getMapHeight(), map.getMapWidth());
		int erosionRadius = 3;

		//====================== BRUSH INIT

		std::vector<std::vector<int>> erosionBrushIndices(mapSize * mapSize);
		std::vector<std::vector<float>> erosionBrushWeights(mapSize * mapSize);

		std::vector<int> xOffsets(erosionRadius * erosionRadius * 4);
		std::vector<int> yOffsets(erosionRadius * erosionRadius * 4);

		std::vector<float> weights(erosionRadius * erosionRadius * 4);

		float weightSum = 0;
		int addIndex = 0;

		for (int i = 0; i < erosionBrushIndices.at(0).size(); i++) {

			int centreX = i % mapSize;
			int centreY = i / mapSize;

			if (centreY <= erosionRadius || centreY >= mapSize - erosionRadius || centreX <= erosionRadius + 1 || centreX >= mapSize - erosionRadius) {
				weightSum = 0;
				addIndex = 0;
				for (int y = -erosionRadius; y <= erosionRadius; y++) {

					for (int x = -erosionRadius; x <= erosionRadius; x++) {

						float sqrDst = x * x + y * y;
						if (sqrDst < erosionRadius * erosionRadius) {
							int coordX = centreX + x;
							int coordY = centreY + y;

							if (coordX >= 0 && coordX < mapSize && coordY >= 0 && coordY < mapSize) {

								float weight = 1 - sqrt(sqrDst) / erosionRadius;
								weightSum += weight;
								weights.assign(addIndex, weight);
								xOffsets.assign(addIndex, x);
								yOffsets.assign(addIndex, y);
								addIndex++;

							}
						}
					}
				}
			}

			int numEntries = addIndex;

			erosionBrushIndices.assign(i, std::vector<int>(numEntries));
			erosionBrushWeights.assign(i, std::vector<float>(numEntries));

			for (int j = 0; j < numEntries; j++) {
				erosionBrushIndices.at(i).assign(j, (yOffsets[j] + centreY) * mapSize + xOffsets[j] + centreX);
				erosionBrushWeights.at(i).assign(j, weights[j] / weightSum);
			}
		}

		//============================================================================================//

		unsigned int maxDropletLifetime = 30;


		float inertia = 0.05f;
		float sedimentCapacityFactor = 4.f;
		float minSedimentCapacity = 0.01f;

		float erodeSpeed = .3f;
		float depositSpeed = .3f;
		float evaporateSpeed = .1f;
		float gravity = 4;

		float initialWaterVolume = 1;
		float initialSpeed = 1;

		float scale = (*(ConcreteHeightMap*)&heightmap).getMaximumHeight();


		int mapWidth = heightmap.getMapWidth();
		float* values = new float[mapWidth * heightmap.getMapHeight()];
		ConcreteHeightMap res = ConcreteHeightMap(heightmap.getMapWidth(), heightmap.getMapHeight(), values);

		for (int y = 0; y < heightmap.getMapHeight(); y++) {
			for (int x = 0; x < heightmap.getMapWidth(); x++) {
				res.setHeightAt(x, y, heightmap.getHeight(x, y));
			}
		}

		// Create water droplet at random point
		
		for (int interation = 0; interation < nbDroplets; interation++) {


			float posX = std::min(rand() % map.getMapWidth(), map.getMapWidth()-2);
			float posY = std::min(rand() % map.getMapHeight(), map.getMapHeight()-2);

			float oldheight = map.getHeight(posX, posY);

			float dirX = 0;
			float dirY = 0;

			float speed = initialSpeed;
			float water = initialWaterVolume;

			float sediment = 0;

			for (int lifetime = 0; lifetime < maxDropletLifetime; lifetime++) {

				// Compute droplet height + direction of flow with bilerp of surrounding height (p8?)


				int nodeX = (int)posX;
				int nodeY = (int)posY;

				int dropletID = nodeY * mapSize + nodeX;

				float cellOffsetX = posX - nodeX;
				float cellOffsetY = posY - nodeY;

				//droplet.height = map.getHeight(droplet.position.x, droplet.position.y); // Multiply by the terrain hight ?

				glm::vec3 heightAndGradient = computeHeightGradientOfCell(map, nodeX, nodeY); // returns {height, g(X), g(Y)}
				

				// Update droplets pos and dir
			
				

				dirX = (dirX * inertia - heightAndGradient.y * (1 - inertia));
				dirY = (dirY * inertia - heightAndGradient.z * (1 - inertia));

				glm::vec2 normalizedDirection = glm::normalize(glm::vec2(dirX, dirY));

				dirX = normalizedDirection.x;
				dirY = normalizedDirection.y;

				posX += dirX;
				posY += dirY;

				// Break if the droplet is not valid (outside of the map or no mvmnt)
				if ((dirX == 0 && dirY == 0) || posX < 0 || posX >= map.getMapWidth() - 1 ||posY < 0 || posY >= map.getMapHeight() - 1) {
					break;
				}

				// find the droplets new height and compute deltaHeight

				float newHeight = computeHeightGradientOfCell(map, posX, posY).x;

				float deltaHeight = newHeight - heightAndGradient.x;

				// Calculate sediment capacity

				float sedimentCapacity = std::max(-deltaHeight * speed * water * sedimentCapacityFactor, minSedimentCapacity); // apparently this should work too


				// If carrying more sediment than capacity or droplet is going up a slope:
				if (sediment > sedimentCapacity || deltaHeight > 0) {

						// deposit a fraction of the sediment to the surrounding nodes (with bilerp)
						
																		// DO NOT DROP MORE THAN DELTAHEIGHT
						float amountToDeposit = (deltaHeight > 0) ? std::min(deltaHeight, sediment) : (sediment - sedimentCapacity) * depositSpeed;
						sediment -= amountToDeposit;

						/*
						map.addHeightAt(nodeX,		nodeY,		amountToDeposit * (1 - cellOffsetX)	* (1 - cellOffsetY));
						map.addHeightAt(nodeX+1,	nodeY,		amountToDeposit * cellOffsetX		* (1 - cellOffsetY));
						map.addHeightAt(nodeX,		nodeY+1,	amountToDeposit	* (1 - cellOffsetX)	* cellOffsetY);
						map.addHeightAt(nodeX+1,	nodeY+1,	amountToDeposit	* cellOffsetX		* cellOffsetY);
						*/

						map[dropletID] += amountToDeposit * (1 - cellOffsetX) * (1 - cellOffsetY);
						map[dropletID + 1] += amountToDeposit * cellOffsetX * (1 - cellOffsetY);
						map[dropletID + mapSize] += amountToDeposit * (1 - cellOffsetX) * cellOffsetY;
						map[dropletID + mapSize + 1] += amountToDeposit * cellOffsetX * cellOffsetY;



				}

				// Else $
				else {
					// Erode a fraction of the droplets remaining capacity from the ground
					// dont erode more than deltaHeight

					float amountToErode = std::min((sedimentCapacity - sediment) * erodeSpeed, -deltaHeight);

					// shitty brush thing, wtf does it mean 
					// TODO droplet radius

					for (int brushPointIndex = 0; brushPointIndex < erosionBrushIndices.at(dropletID).size(); brushPointIndex++) {

						int nodeIndex = erosionBrushIndices.at(dropletID).at(brushPointIndex);
						float weighedErodeAmount = amountToErode * erosionBrushWeights.at(dropletID).at(brushPointIndex);

						float deltaSediment = (map[nodeIndex] < weighedErodeAmount) ? map[nodeIndex] : weighedErodeAmount;
						map[nodeIndex] -= deltaSediment;
						sediment += deltaSediment;
					}




				}


				// update dropplets speed based on deltaheight
				// evaporate a fraction of the water of the droplet

				speed = std::sqrt(speed * speed + deltaHeight * gravity);
				water *= (1 - evaporateSpeed);


			}
			
			float newheight = map.getHeight(posX, posY);
			float delta = (newheight - oldheight) *scale;

			float a = heightmap.getHeight(posX, posY);

			heightmap.addHeightAt(posX, posY, delta); // magic number todo get the max of the heightmap
			float b = heightmap.getHeight(posX, posY);
			//std::cout << a - b << std::endl;

		}
		


	}

	//============================================================================================//
	//============================================================================================//
	//============================================================================================//
	//============================================================================================//
	//============================================================================================//
	//============================================================================================//


private:

	static glm::vec3 computeHeightGradientOfCell(const HeightMap& heightmap, float posX, float posY) {

		int coordX = (int)posX;
		int coordY = (int)posY;

		float x = posX - coordX;
		float y = posY - coordY;

		// Get the gradient from the surroundings points
		// (0,0) is North west, (1,1) is south east

		float heightNW = heightmap.getHeight(coordX,		coordY);
		float heightNE = heightmap.getHeight(coordX + 1,	coordY);
		float heightSW = heightmap.getHeight(coordX,		coordY + 1);;
		float heightSE = heightmap.getHeight(coordX + 1,	coordY + 1);

		// Compute the gradient (p.9)

		float gradientX = (heightNE - heightNW) * (1 - y) + (heightSE - heightSW) * y;
		float gradientY = (heightSW - heightNW) * (1 - x) + (heightSE - heightNE) * x;

		// Apparently this computes the new height
		// Something like bilerp all the heights

		float height = heightNW * (1 - x) * (1 - y) + heightNE * x * (1 - y) + heightSW * (1 - x) * y + heightSE * x * y;

		return { height, gradientX, gradientY };
	}

	static void InitializeBrushIndices(int mapSize, int radius) {
		std::vector<std::vector<int>> erosionBrushIndices(mapSize * mapSize);
		std::vector<std::vector<float>> erosionBrushWeights(mapSize * mapSize);

		std::vector<int> xOffsets(radius * radius * 4);
		std::vector<int> yOffsets(radius * radius * 4);

		std::vector<float> weights(radius * radius * 4);

		float weightSum = 0;
		int addIndex = 0;

		for (int i = 0; i < erosionBrushIndices.at(0).size() ; i++) {

			int centreX = i % mapSize;
			int centreY = i / mapSize;

			if (centreY <= radius || centreY >= mapSize - radius || centreX <= radius + 1 || centreX >= mapSize - radius) {
				weightSum = 0;
				addIndex = 0;
				for (int y = -radius; y <= radius; y++) {

					for (int x = -radius; x <= radius; x++) {

						float sqrDst = x * x + y * y;
						if (sqrDst < radius * radius) {
							int coordX = centreX + x;
							int coordY = centreY + y;

							if (coordX >= 0 && coordX < mapSize && coordY >= 0 && coordY < mapSize) {

								float weight = 1 - sqrt(sqrDst) / radius;
								weightSum += weight;
								weights.assign(addIndex, weight);
								xOffsets.assign(addIndex, x);
								yOffsets.assign(addIndex, y);
								addIndex++;

							}
						}
					}
				}
			}

			int numEntries = addIndex;
			
			erosionBrushIndices.assign(i, std::vector<int>(numEntries));
			erosionBrushWeights.assign(i, std::vector<float>(numEntries));			

			for (int j = 0; j < numEntries; j++) {
				erosionBrushIndices.at(i).assign(j, (yOffsets[j] + centreY) * mapSize + xOffsets[j] + centreX);
				erosionBrushWeights.at(i).assign(j, weights[j] / weightSum);
			}
		}
	}











};

