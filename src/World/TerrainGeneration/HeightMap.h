#pragma once
#include <memory>
#include "../../Utils/Mathf.h"


class HeightMap
{


private:
  unsigned int m_width, m_height;
  float *m_heightValues;
public:


	HeightMap() : 
		m_heightValues(nullptr),
		m_width(0),
	  m_height(0) {
	}
	~HeightMap() {

  }

  HeightMap(HeightMap&& moved) noexcept {

	  m_width = moved.m_width;
	  m_height = moved.m_height;
	  m_heightValues = moved.m_heightValues;
	  moved.m_heightValues = nullptr;

  }

  HeightMap& operator=(HeightMap&& moved) noexcept {
	  this->~HeightMap();
	  new (this)HeightMap(std::move(moved));
	  return *this;
  }

  HeightMap(const HeightMap& other) {

	  m_width = other.m_width;
	  m_height = other.m_height;
	  m_heightValues = new float[m_width * m_height];

	  for (unsigned int i = 0; i < m_width * m_height; i++) {

		  m_heightValues[i] = other.m_heightValues[i];

	  }
  }
  HeightMap& operator=(const HeightMap& other) {

	  delete[] m_heightValues;
	  m_width = other.m_width;
	  m_height = other.m_height;
	  m_heightValues = new float[m_width * m_height];

	  for (unsigned int i = 0; i < m_width * m_height; i++) {

		  m_heightValues[i] = other.m_heightValues[i];

	  }


	  return *this;

	  
  }

  void setHeights(unsigned int width, unsigned int height, float *heights);

  unsigned int getMapWidth() const { return m_width; }
  unsigned int getMapHeight() const { return m_height; }
  float getHeight(int x, int y) const;
  float getHeightLerp(float x, float y) const;
  const float *getBackingArray() const;
};

class HeightMapView {

private:

	const HeightMap* originMap;
	glm::ivec2 originPoint, subMapSize;

public:
	HeightMapView(const HeightMap& heightMap, const glm::ivec2& origin, const glm::ivec2& size)
	{
		originMap = &heightMap;
		subMapSize = size;
		originPoint = origin;
	}
	
	float getHeight(int x, int y) const {
		int projectedX, projectedY;

		projectedX = originPoint.x + x;
		projectedY = originPoint.y + y;

		return originMap->getHeight(projectedX, projectedY);
	}
	const HeightMap& getOriginMap()  const { return *originMap; }
	glm::vec2 getOriginPoint() const { return originPoint; }

	unsigned int getMapWidth() const { return subMapSize.x+1; } // TODO the +1 should not be here (my (albin) fault!)
	unsigned int getMapHeight() const { return subMapSize.y+1; }

};