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
  virtual void addHeightAt(int x, int y, float delta) {}
  float getHeightLerp(float x, float y) const;

};

class ConcreteHeightMap : public HeightMap
{
private:
  float *m_heightValues;

public:
  float max = -INFINITY; // shitty parameter that is only worth something when the map is generated using normalize(). this is the poopiest thing ever
  ConcreteHeightMap();
  ConcreteHeightMap(unsigned int width, unsigned int height, float *heights);
  ConcreteHeightMap(ConcreteHeightMap &&moved) noexcept;
  ConcreteHeightMap &operator=(ConcreteHeightMap &&moved) noexcept;
  ConcreteHeightMap(const ConcreteHeightMap &other);
  ConcreteHeightMap &operator=(const ConcreteHeightMap &other);
  ~ConcreteHeightMap();

  float& operator[](int pos) { return m_heightValues[pos]; }
  void setHeights(unsigned int width, unsigned int height, float *heights);
  void setHeightAt(int x, int y, float value)  { // move this out
	  m_heightValues[y * m_width + x] = value;
  }
  void addHeightAt(int x, int y, float delta)override {

	  m_heightValues[y * m_width + x] += delta;

  }

  const float* getValues() const { return m_heightValues; } // dangereux

  float getHeight(int x, int y) const override;
  const float *getBackingArray() const;

  float getMaximumHeight() const {

      float max = -INFINITY;
      for (int y = 0; y < m_height; y++) {
          for (int x = 0; x < m_width; x++) {

              max = std::max(getHeight(x, y), max);

          }
      }

      return max;
  }

  static ConcreteHeightMap normalize(const HeightMap& heightmap);
  static ConcreteHeightMap scale(const ConcreteHeightMap& heightmap, float scale); // assume that the heightmap is normalized 

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