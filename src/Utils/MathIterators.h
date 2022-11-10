#pragma once

#include <glm/glm.hpp>

#include "Mathf.h"

/* --------------------------------------------------------------------------
 * Iterators, go to the end of the file to see the full list
 * -------------------------------------------------------------------------- */

class SpiralGridIterator {
private:
  glm::ivec2 m_origin;
  glm::ivec2 m_currentPosition;
  glm::ivec2 m_currentDirection;
public:
  constexpr SpiralGridIterator(glm::ivec2 origin, glm::ivec2 point)
    : m_origin(origin), m_currentPosition(point), m_currentDirection(0, 1)
  {
  }

  constexpr glm::ivec2 operator*() const
  {
    return m_origin + m_currentPosition;
  }

  constexpr SpiralGridIterator &operator++()
  {
    // https://stackoverflow.com/questions/398299/looping-in-a-spiral
    m_currentPosition += m_currentDirection;
    if ((m_currentPosition.x == m_currentPosition.y) ||
        (m_currentPosition.y < 0 && m_currentPosition.y == -m_currentPosition.x) ||
        (m_currentPosition.y > 0 && m_currentPosition.y == 1 - m_currentPosition.x)) {
      int dx = m_currentDirection.x;
      m_currentDirection.x = m_currentDirection.y;
      m_currentDirection.y = -dx;
    }
    return *this;
  }

  friend constexpr bool operator==(const SpiralGridIterator &a, const SpiralGridIterator &b) { return a.m_currentPosition == b.m_currentPosition; }
  friend constexpr bool operator!=(const SpiralGridIterator &a, const SpiralGridIterator &b) { return a.m_currentPosition != b.m_currentPosition; }
};

class SpiralGridIterable {
private:
  glm::ivec2 m_origin;
  glm::ivec2 m_lastPoint;
  size_t m_cellCount;

public:
  constexpr SpiralGridIterable(glm::ivec2 origin, glm::ivec2 lastPoint, size_t cellCount)
    : m_origin(origin), m_lastPoint(lastPoint), m_cellCount(cellCount)
  {
  }

  constexpr SpiralGridIterator begin() const { return SpiralGridIterator(m_origin, { 0,0 }); }
  constexpr SpiralGridIterator end() const { return SpiralGridIterator(m_origin, m_lastPoint); }

  constexpr size_t getCount() const { return m_cellCount; }
};



class CircularGridIterator {
private:
  glm::ivec2 m_center;
  glm::ivec2 m_current;
  float      m_d, m_r;

public:
  constexpr CircularGridIterator(glm::ivec2 center, glm::ivec2 currentPosition, float r)
    : m_center(center), m_r(r), m_current(currentPosition), m_d(0)
  {
  }

  constexpr glm::ivec2 operator*() const
  {
    return m_center + m_current;
  }

  constexpr CircularGridIterator &operator++()
  {
    if (--m_current.y < -m_d) {
      m_current.x++;
      m_d = (float)Mathf::sqrt(std::max(0.f, m_r * m_r - m_current.x * m_current.x));
      m_current.y = (int)m_d;
    }
    return *this;
  }

  friend constexpr bool operator==(const CircularGridIterator &a, const CircularGridIterator &b) { return a.m_current == b.m_current; }
  friend constexpr bool operator!=(const CircularGridIterator &a, const CircularGridIterator &b) { return a.m_current != b.m_current; }
};

class CircularGridIterable {
private:
  glm::ivec2 m_center;
  float      m_radius;

public:
  constexpr CircularGridIterable(glm::ivec2 center, float radius)
    : m_center(center), m_radius(radius)
  {
    assert(radius > 0);
  }

  constexpr CircularGridIterator begin() const { return CircularGridIterator(m_center, { -m_radius, 0 }, m_radius); }
  constexpr CircularGridIterator end() const { return CircularGridIterator(m_center, { +m_radius, 0 }, m_radius); }
};



class DiamondGridIterator {
private:
  glm::ivec2 m_center;
  glm::ivec2 m_current;
  int        m_diameter, m_d;

public:
  constexpr DiamondGridIterator(glm::ivec2 center, glm::ivec2 currentPosition, int diameter)
    : m_center(center), m_current(currentPosition), m_diameter(diameter), m_d(0)
  {
  }

  constexpr glm::ivec2 operator*() const
  {
    return m_center + m_current;
  }

  constexpr DiamondGridIterator &operator++()
  {
    if (--m_current.y < -m_d) {
      m_current.x++;
      m_d = m_diameter - glm::abs(m_current.x) - 1;
      m_current.y = m_d;
    }
    return *this;
  }

  friend constexpr bool operator==(const DiamondGridIterator &a, const DiamondGridIterator &b) { return a.m_current == b.m_current; }
  friend constexpr bool operator!=(const DiamondGridIterator &a, const DiamondGridIterator &b) { return a.m_current != b.m_current; }
};

class DiamondGridIterable {
private:
  glm::ivec2 m_center;
  int        m_radius;
public:
  constexpr DiamondGridIterable(glm::ivec2 center, int radius)
    : m_center(center), m_radius(radius)
  {
    assert(radius > 0);
  }

  constexpr size_t getCount() const { return 1 + 2ull * m_radius * (m_radius - 1ull); }

  constexpr DiamondGridIterator begin() const { return DiamondGridIterator(m_center, { -m_radius+1, 0 }, m_radius); }
  constexpr DiamondGridIterator end() const { return DiamondGridIterator(m_center, { +m_radius, -1 }, m_radius); }
};


/* --------------------------------------------------------------------------
 * Iterators, generally useful to replace for(xy) loops.
 * General usage:
 *   for(gml::ivec2 p : Iterators::iterateOverSquare({ 0,0 }, 2);
 * Iterator constructors are public but should not be used directly.
 * -------------------------------------------------------------------------- */
namespace Iterators {

/* --------------------------------------------------------------------------
 * Spiral iterator:
 *    8  1--2
 *    |  |  |
 *    7  0  3
 *    |     |
 *    6--5--4
 * Very useful for dealing with chunks or finding the closest objets to smt
 * -------------------------------------------------------------------------- */
inline constexpr SpiralGridIterable iterateOverSquare(glm::ivec2 center, int squareSize)
{
  assert(squareSize > 0);
  if (squareSize % 2 == 0)
    return SpiralGridIterable(center, { squareSize / 2, -squareSize / 2 }, squareSize * squareSize);
  else
    return SpiralGridIterable(center, { -squareSize / 2, squareSize / 2 + 1 }, squareSize * squareSize);
}

/* --------------------------------------------------------------------------
 * Cirtcular iterator:
 *    ....
 *  ........
 *  ........
 * ..........
 * ..........
 * ..........
 * ..........
 *  ........
 *  ........
 *    ....
 * Ordered from left to right, top to bottom, use to cover a circle but do not
 * assume that the first points are near the center.
 * -------------------------------------------------------------------------- */
inline constexpr CircularGridIterable iterateOverCircle(glm::ivec2 center, float radius)
{
  return CircularGridIterable(center, radius);
}

/* --------------------------------------------------------------------------
 * Diamond iterator:
 *   .
 *  ...
 * ......
 *  ...
 *   .
 * Ordered from left to right, top to bottom, use to cover a diamond but do not
 * assume that the first points are near the center.
 * -------------------------------------------------------------------------- */
inline constexpr DiamondGridIterable iterateOverDiamond(glm::ivec2 center, int radius)
{
  return DiamondGridIterable(center, radius);
}


}