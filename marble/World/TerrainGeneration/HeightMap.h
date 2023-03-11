#pragma once

#include <glm/glm.hpp>

namespace Noise {

/**
* An heightmap is a "2D continuous grid", it maps every xy position to a height (z) value.
* 
* Concrete implementations may have size requirements that prevent from sampling outside
* of some bounds, this implementation provides width and height fields that users should
* respect (not sample outside of).
* 
* Heightmaps implementations are not required to provide height values for every xy point
* but rather for xy points with round coordinates (#getHeight), these values are then used 
* to lerp between when a non-round point is sampled (#getHeightLerp).
*/
class HeightMap {
protected:
  unsigned int m_width, m_height;
public:
  HeightMap(unsigned int width, unsigned int height);
  virtual ~HeightMap() = default;

  unsigned int getMapWidth() const { return m_width; }
  unsigned int getMapHeight() const { return m_height; }

  bool isInBounds(int x, int y) const;
  bool isInBounds(float x, float y) const;

  virtual float getHeight(int x, int y) const = 0;
  float getHeightLerp(float x, float y) const;
  float operator()(float x, float y) const { return getHeightLerp(x, y); }
};

/**
* Standard implementation of Heightmap, when provided with a grid of
* heights of a certain size, heights are sampled and lerped between.
* Heigth values can be modified after construction but not resized.
*/
class ConcreteHeightMap : public HeightMap {
private:
  float *m_heightValues;

public:
  ConcreteHeightMap();
  ConcreteHeightMap(unsigned int width, unsigned int height, float *heights);
  ConcreteHeightMap(ConcreteHeightMap &&moved) noexcept;
  ConcreteHeightMap &operator=(ConcreteHeightMap &&moved) noexcept;
  ConcreteHeightMap(const ConcreteHeightMap &other);
  ConcreteHeightMap &operator=(const ConcreteHeightMap &other);
  ~ConcreteHeightMap();

  static ConcreteHeightMap extractConcreteMap(const HeightMap &from, glm::vec2 extractedOrigin, glm::vec2 extractedSize, glm::uvec2 extractionSize);

  float &operator[](int pos) { assert(pos >= 0 && pos < (int)(m_width * m_height)); return m_heightValues[pos]; }
  void setHeights(unsigned int width, unsigned int height, float *heights);
  inline void setHeightAt(int x, int y, float value) { m_heightValues[y * m_width + x] = value; }
  inline void addHeightAt(int x, int y, float delta) { m_heightValues[y * m_width + x] += delta; }

  const float *getBackingArray() const { return m_heightValues; } // unsafe

  float getHeight(int x, int y) const override;
};

} // !namespace Terrain