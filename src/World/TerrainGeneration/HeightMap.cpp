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

ConcreteHeightMap ConcreteHeightMap::normalize(const HeightMap& heightmap) {

    float* values = new float[heightmap.getMapWidth() * heightmap.getMapHeight()];


    float max = -INFINITY;
    float min = INFINITY;

    for (int y = 0; y < heightmap.getMapHeight(); y++) {
        for (int x = 0; x < heightmap.getMapWidth(); x++) {

            max = std::max(heightmap.getHeight(x, y), max);
            min = std::min(heightmap.getHeight(x, y), min);

        }
    }

    float dist = 1.f/(max - min);

    for (int y = 0; y < heightmap.getMapHeight(); y++) {
        for (int x = 0; x < heightmap.getMapWidth(); x++) {

            float norm = (heightmap.getHeight(x, y) - min) * dist;
            values[y * heightmap.getMapWidth() + x] = norm;

        }
    }

    ConcreteHeightMap res = ConcreteHeightMap(heightmap.getMapWidth(), heightmap.getMapHeight(), values);
    res.max = max;
    return res; 



}

ConcreteHeightMap ConcreteHeightMap::scale(const ConcreteHeightMap& heightmap, float scale) {

    int mapWidth = heightmap.getMapWidth();
    float* values = new float[mapWidth * heightmap.getMapHeight()];


    for (int y = 0; y < heightmap.getMapHeight(); y++) {
        for (int x = 0; x < heightmap.getMapWidth(); x++) {

            values[y* mapWidth + x] = heightmap.getHeight(x, y) * scale;

        }
    }

    ConcreteHeightMap res = ConcreteHeightMap(heightmap.getMapWidth(), heightmap.getMapHeight(), values);
    return res; 


}

//==========================================================================================================================================//
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
