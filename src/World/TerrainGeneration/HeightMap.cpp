#include "HeightMap.h"
#include "../../Utils/Mathf.h"




void HeightMap::setHeights(unsigned int width, unsigned int height, float *heights)
{
  m_width = width;
  m_height = height;
  m_heightValues = heights;
}

void HeightMap::setHeightAt(int x, int y, float value) {

    assert(x <= m_width && x >= 0);
    assert(y <= m_height && y >= 0);
    assert(value <= 1 && value >= 0);

    m_heightValues[y * m_width + x] = value;


}

void HeightMap::addHeightAt(int x, int y, float delta) {

    float prev = getHeight(x, y);
    float value = (delta < 0) ? std::min(0.f, prev + delta) : std::max(1.f, prev + delta);

    setHeightAt(x, y, value);



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
