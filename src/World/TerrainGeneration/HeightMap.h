#pragma once

class HeightMap
{
private:
  unsigned int m_width, m_height;
  float *m_heightValues;
public:
  HeightMap();
  ~HeightMap();

  HeightMap(const HeightMap &) = delete;
  HeightMap &operator=(const HeightMap &) = delete;

  void setHeights(unsigned int width, unsigned int height, float *heights);

  unsigned int getMapWidth() const { return m_width; }
  unsigned int getMapHeight() const { return m_height; }
  float getHeight(int x, int y) const;
  float getHeightLerp(float x, float y) const;
  const float *getBackingArray() const;
};

