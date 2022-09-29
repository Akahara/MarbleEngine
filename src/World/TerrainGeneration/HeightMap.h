#pragma once

class HeightMap
{
private:
  size_t m_width, m_height;
  float *m_heightValues;
public:
  HeightMap();
  ~HeightMap();

  HeightMap(const HeightMap &) = delete;
  HeightMap &operator=(const HeightMap &) = delete;

  void setHeights(size_t width, size_t height, float *heights);

  float getHeight(int x, int y) const;
  float getHeightLerp(float x, float y) const;
  const float *getBackingArray() const;
};

