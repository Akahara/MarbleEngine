#pragma once
#include <memory>
#include "../../Utils/Mathf.h"


class HeightMap
{
protected:
  unsigned int m_width, m_height;
public:
  HeightMap(unsigned int width, unsigned int height);
  virtual ~HeightMap() = default;

  unsigned int getMapWidth() const { return m_width; }
  unsigned int getMapHeight() const { return m_height; }

  bool isInBounds(int x, int y) const;
  virtual float getHeight(int x, int y) const = 0;
  float getHeightLerp(float x, float y) const;
};

class ConcreteHeightMap : public HeightMap
{
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

  void setHeights(unsigned int width, unsigned int height, float *heights);
  void setHeightAt(int x, int y, float value); // for erosion
  void addHeightAt(int x, int y, float delta);

  const float* getValues() const { return m_heightValues; } // dangereux

  float getHeight(int x, int y) const override;
  const float *getBackingArray() const;
};

class HeightMapView : public HeightMap {
private:
  const HeightMap* m_originMap;
  glm::ivec2 m_originPoint;

public:
  HeightMapView(const HeightMap &heightMap, const glm::ivec2 &origin, const glm::ivec2 &size);

  float getHeight(int x, int y) const override;

  //const HeightMap& getOriginMap()  const { return *m_originMap; }
  //glm::vec2 getOriginPoint() const { return m_originPoint; }
};