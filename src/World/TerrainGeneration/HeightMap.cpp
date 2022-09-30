#include "HeightMap.h"

#include "../../Utils/Mathf.h"
#include "Noise.h"

HeightMap::HeightMap()
  : m_width(0),
  m_height(0),
  m_heightValues(nullptr)
{
}

HeightMap::~HeightMap()
{
  delete m_heightValues;
}

void HeightMap::setHeights(unsigned int width, unsigned int height, float *heights)
{
  delete m_heightValues;
  m_width = width;
  m_height = height;
  m_heightValues = heights;
}

float HeightMap::getHeight(int x, int y) const
{
  return m_heightValues[Mathf::positiveModulo(x, m_width) + Mathf::positiveModulo(y, m_height) * m_width];
}

float HeightMap::getHeightLerp(float x, float y) const
{
  int x1 = (int)x;
  int y1 = (int)y;
  float a1 = getHeight(x1,   y1  );
  float a2 = getHeight(x1+1, y1  );
  float a3 = getHeight(x1,   y1+1);
  float a4 = getHeight(x1+1, y1+1);
  // bilinear lerp
  return Mathf::lerp(
    Mathf::lerp(a1, a2, x-x1),
    Mathf::lerp(a3, a4, x-x1),
    y-y1);
}

const float *HeightMap::getBackingArray() const
{
  return m_heightValues;
}
