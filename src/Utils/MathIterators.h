#pragma once

#include <glm/glm.hpp>

/* --------------------------------------------------------------------------
 * Spiral iterator, iterates over a grid in a spiral fashion:
 * 
 *    8  1--2
 *    |  |  |
 *    7  0  3
 *    |     |
 *    6--5--4
 * 
 * Very useful for dealing with chunks or finding the closest objets to smt
 * 
 * IE: for(glm::ivec2 point : SpiralGridIterable::iterateOverSquare({0,0}, 3)
 * -------------------------------------------------------------------------- */
class SpiralGridIterator {
private:
  glm::ivec2 m_origin;
  glm::ivec2 m_currentPosition;
  glm::ivec2 m_currentDirection;
public:
  SpiralGridIterator(glm::ivec2 origin, glm::ivec2 point)
    : m_origin(origin), m_currentPosition(point), m_currentDirection(0, 1)
  {
  }

  glm::ivec2 operator*() const
  {
    return m_origin + m_currentPosition;
  }

  SpiralGridIterator &operator++()
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

  friend bool operator==(const SpiralGridIterator &a, const SpiralGridIterator &b) { return a.m_currentPosition == b.m_currentPosition; }
  friend bool operator!=(const SpiralGridIterator &a, const SpiralGridIterator &b) { return a.m_currentPosition != b.m_currentPosition; }
};

class SpiralGridIterable {
private:
  glm::ivec2 m_origin;
  glm::ivec2 m_lastPoint;

  SpiralGridIterable(glm::ivec2 origin, glm::ivec2 lastPoint)
    : m_origin(origin), m_lastPoint(lastPoint)
  {
  }
public:
  SpiralGridIterator begin() const { return SpiralGridIterator(m_origin, { 0,0 }); }
  SpiralGridIterator end() const { return SpiralGridIterator(m_origin, m_lastPoint); }

  static SpiralGridIterable iterateOverSquare(glm::ivec2 center, int squareSize)
  {
    assert(squareSize > 0);
    if (squareSize % 2 == 0)
      return SpiralGridIterable(center, { squareSize / 2, -squareSize / 2 });
    else
      return SpiralGridIterable(center, { -squareSize / 2, squareSize / 2 + 1 });
  }

  static SpiralGridIterable iterateUpToPoint(glm::ivec2 startingPoint, glm::ivec2 destination)
  {
    return SpiralGridIterable(startingPoint, destination);
  }
};