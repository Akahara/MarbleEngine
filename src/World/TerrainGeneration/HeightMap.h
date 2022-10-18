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

  HeightMap(HeightMap&& moved) {

	  m_width = moved.m_width;
	  m_height = moved.m_height;
	  m_heightValues = moved.m_heightValues;
	  moved.m_heightValues = nullptr;

  }

  HeightMap& operator=(HeightMap&& moved) {
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
	glm::vec2 originPoint, subMapSize;

public:
	HeightMapView(const HeightMap& heightMap, const glm::vec2& origin, const glm::vec2& size)

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

	unsigned int getMapWidth() const { return subMapSize.x+1; }
	unsigned int getMapHeight() const { return subMapSize.y+1; }

};