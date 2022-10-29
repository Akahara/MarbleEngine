#include "HeightMap.h"

#include "../../Utils/Mathf.h"

HeightMap::HeightMap(unsigned int width, unsigned int height)
  : m_width(width), m_height(height)
{
}

bool HeightMap::isInBounds(int x, int y) const
{
  return (x >= 0 && x < (int)m_width) && (y >= 0 && y < (int)m_height);
}

<<<<<<< HEAD
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
=======
    //assert(x <= m_width && x >= 0);
    //assert(y <= m_height && y >= 0);
    //assert(value <= 1 && value >= 0);

    //m_heightValues[y * m_width + x] = value;
    m_heightValues[Mathf::positiveModulo(x, m_width) + Mathf::positiveModulo(y, m_height) * m_width] = value;
>>>>>>> feat : end of the algorithm, doesnt work ! too bad !

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
  memcpy_s(m_heightValues, arraySize * sizeof(float), other.m_heightValues, arraySize * sizeof(float));
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
  assert(isInBounds(x, y));
  return m_heightValues[x + y * m_width];
}

const float *ConcreteHeightMap::getBackingArray() const
{
  return m_heightValues;
}

HeightMapView::HeightMapView(const HeightMap &originMap, const glm::ivec2 &origin, const glm::ivec2 &size)
  : HeightMap(size.x, size.y),
  m_originMap(&originMap),
  m_originPoint(origin)
{
  assert(originMap.isInBounds(origin.x, origin.y));
  assert(originMap.isInBounds(origin.x + size.x-1, origin.y + size.y-1));
}

float HeightMapView::getHeight(int x, int y) const
{
  assert(isInBounds(x, y));
  return m_originMap->getHeight(m_originPoint.x + x, m_originPoint.y + y);
}
