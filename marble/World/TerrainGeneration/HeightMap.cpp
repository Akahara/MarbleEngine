#include "HeightMap.h"

#include "../../Utils/Mathf.h"
#include "Terrain.h"
#include <cstring>

namespace Noise {

HeightMap::HeightMap(unsigned int width, unsigned int height)
  : m_width(width), m_height(height)
{
}

bool HeightMap::isInBounds(int x, int y) const
{
  return (x >= 0 && x < (int)m_width) && (y >= 0 && y < (int)m_height);
}

bool HeightMap::isInBounds(float x, float y) const
{
  return (x >= 0 && x < (int)m_width) && (y >= 0 && y < (int)m_height);
}

float HeightMap::getHeightLerp(float x, float y) const
{
  int x1 = (int)x;
  int y1 = (int)y;
  float a1 = getHeight(x1, y1);
  float a2 = getHeight(x1 + 1, y1);
  float a3 = getHeight(x1, y1 + 1);
  float a4 = getHeight(x1 + 1, y1 + 1);
  // bilinear lerp
  return Mathf::lerp(
    Mathf::lerp(a1, a2, x - x1),
    Mathf::lerp(a3, a4, x - x1),
    y - y1);
}

ConcreteHeightMap::ConcreteHeightMap()
  : HeightMap(0, 0), m_heightValues(nullptr)
{
}

ConcreteHeightMap::ConcreteHeightMap(unsigned int width, unsigned int height, float *heights)
  : HeightMap(width, height), m_heightValues(heights)
{
}

ConcreteHeightMap::ConcreteHeightMap(ConcreteHeightMap &&moved) noexcept
  : HeightMap(moved.getMapWidth(), moved.getMapHeight())
{
  m_heightValues = moved.m_heightValues;
  moved.m_heightValues = nullptr;
}

ConcreteHeightMap &ConcreteHeightMap::operator=(ConcreteHeightMap &&moved) noexcept
{
  this->~ConcreteHeightMap();
  new (this)ConcreteHeightMap(std::move(moved));
  return *this;
}

ConcreteHeightMap::ConcreteHeightMap(const ConcreteHeightMap &other)
  : HeightMap(other.getMapWidth(), other.getMapHeight())
{
  size_t arraySize = (size_t)m_width * m_height;
  m_heightValues = new float[arraySize];
  memcpy(m_heightValues, other.m_heightValues, arraySize * sizeof(float));
}

ConcreteHeightMap &ConcreteHeightMap::operator=(const ConcreteHeightMap &other)
{
  this->~ConcreteHeightMap();
  new (this)ConcreteHeightMap(other);
  return *this;
}

ConcreteHeightMap::~ConcreteHeightMap()
{
  delete[] m_heightValues;
}

ConcreteHeightMap ConcreteHeightMap::extractConcreteMap(const HeightMap &from, glm::vec2 extractedOrigin, glm::vec2 extractedSize, glm::uvec2 extractionSize)
{
  assert(from.isInBounds(extractedOrigin.x, extractedOrigin.y));
  assert(from.isInBounds(extractedOrigin.x + extractedSize.x, extractedOrigin.y + extractedSize.y));
  unsigned int w = extractionSize.x;
  unsigned int h = extractionSize.x;
  float *heights = new float[w*h];
  for (size_t y = 0; y < h; y++) {
    for (size_t x = 0; x < w; x++) {
      heights[x + y * w] = from.getHeightLerp((float)x/(w-1)*extractedSize.x + extractedOrigin.x, (float)y/(h-1)*extractedSize.y + extractedOrigin.y);
    }
  }
  return ConcreteHeightMap{ w, h, heights };
}

void ConcreteHeightMap::setHeights(unsigned int width, unsigned int height, float *heights)
{
  delete[] m_heightValues;
  m_width = width;
  m_height = height;
  m_heightValues = heights;
}

float ConcreteHeightMap::getHeight(int x, int y) const
{
  assert(m_heightValues != nullptr);
  return isInBounds(x, y) ? m_heightValues[x + y * m_width] : 0;
}

} // !namespace Terrain
