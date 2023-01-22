#pragma once

#include "BoundingSphere.h"


class AABBIterator;

/**
* An AABB (axis aligned bounding box) is a bounding box, a quad that fully
* contains an arbitrary object. It is often used to quickly check if two
* objects overlap, like a cube and a camera viewport - if they don't there
* is no need to render the cube.
*/
class AABB {
private:
  glm::vec3 m_origin;
  glm::vec3 m_size;
public:
  AABB()
    : m_origin{}, m_size{} { }
  AABB(const glm::vec3 &origin, const glm::vec3 &size)
    : m_origin(origin), m_size(size)
  {
    assert(size.x >= 0);
    assert(size.y >= 0);
    assert(size.z >= 0);
  }

  static AABB make_aabb(const glm::vec3 &A, const glm::vec3 &B)
  {
    glm::vec3 origin = glm::min(A, B);
    glm::vec3 size = glm::abs(A - B);
    return AABB(origin, size);
  }

  static AABB makeAABBfromBoundingSphere(const BoundingSphere& sphere) 
  {
    glm::vec3 A = sphere.getCenter() - glm::vec3(sphere.getRadius());
    glm::vec3 B = sphere.getCenter() + glm::vec3(sphere.getRadius());
    return make_aabb(A, B);
  }

  BoundingSphere getBoundingSphere() const
  {
    float r = glm::length(m_size / 2.f);
    glm::vec3 c = m_origin + m_size / 2.f;
    return BoundingSphere(c, r);
  }

  const glm::vec3 &getOrigin() const { return m_origin; }
  const glm::vec3 &getSize() const { return m_size; }

  void scale(const glm::vec3 &factor) { m_size *= factor; }
  void setOrigin(const glm::vec3 &origin) { m_origin = origin; }
  void setSize(const glm::vec3 size) { m_size = size; }

  bool isInBounds(const glm::vec3 &point) const 
  {
    return 
      point.x > m_origin.x && point.x < m_origin.x + m_size.x &&
      point.y > m_origin.y && point.y < m_origin.y + m_size.y &&
      point.z > m_origin.z && point.z < m_origin.z + m_size.z;
  }

  static bool doBoxesOverlap(const AABB &box1, const AABB &box2)
  {
    return
      box1.m_origin.x < box2.m_origin.x + box2.m_size.x && box1.m_origin.x + box2.m_origin.x > box2.m_origin.x &&
      box1.m_origin.y < box2.m_origin.y + box2.m_size.y && box1.m_origin.y + box2.m_origin.y > box2.m_origin.y &&
      box1.m_origin.z < box2.m_origin.z + box2.m_size.z && box1.m_origin.z + box2.m_origin.z > box2.m_origin.z;
  }

  AABBIterator begin() const;
  AABBIterator end()   const;
};

/**
* There are some cases where it is useful to iterate over the vertices
* of an AABB, this class does just that.
* 
* Example usage:
*   AABB aabb;
*   for(glm::vec3 p : aabb) {...}
*/
class AABBIterator {
private:
  const AABB *m_aabb;
  unsigned char m_vertexIndex;
public:
  AABBIterator(const AABB *aabb, unsigned char vertexIndex)
    : m_aabb(aabb), m_vertexIndex(vertexIndex) { }

  glm::vec3 operator*() const {
    glm::vec3 v = m_aabb->getOrigin();
    if (m_vertexIndex & 1) v.x += m_aabb->getSize().x;
    if (m_vertexIndex & 2) v.y += m_aabb->getSize().y;
    if (m_vertexIndex & 4) v.z += m_aabb->getSize().z;
    return v;
  }

  AABBIterator &operator++() { m_vertexIndex++; return *this; }
  AABBIterator operator++(int) { AABBIterator tmp = *this; ++(*this); return tmp; }

  friend bool operator==(const AABBIterator &a, const AABBIterator &b) { return a.m_vertexIndex == b.m_vertexIndex; }
  friend bool operator!=(const AABBIterator &a, const AABBIterator &b) { return a.m_vertexIndex != b.m_vertexIndex; }
};

inline AABBIterator AABB::begin() const { return AABBIterator(this, 0); }
inline AABBIterator AABB::end() const { return AABBIterator(this, 8); }
