#pragma once

#include <glm/glm.hpp>

namespace Terrain {

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
};

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