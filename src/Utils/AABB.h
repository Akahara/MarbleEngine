#pragma once

#include <glm/glm.hpp>

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
    assert(size.x > 0);
    assert(size.y > 0);
    assert(size.z > 0);
  }

  static AABB make_aabb(const glm::vec3 &A, const glm::vec3 &B)
  {
    glm::vec3 origin = glm::min(A, B);
    glm::vec3 size = glm::abs(A - B);
    return AABB(origin, size);
  }

  const glm::vec3 &GetOrigin() const { return m_origin; }
  const glm::vec3 &GetSize() const { return m_size; }

  bool IsInBounds(const glm::vec3 &point) const 
  {
    return 
      point.x > m_origin.x && point.x < m_origin.x + m_size.x &&
      point.y > m_origin.y && point.y < m_origin.y + m_size.y &&
      point.z > m_origin.z && point.z < m_origin.z + m_size.z;
  }

  static bool DoBoxOverlap(const AABB &box1, const AABB &box2)
  {
    return
      box1.m_origin.x < box2.m_origin.x + box2.m_size.x && box1.m_origin.x + box2.m_origin.x > box2.m_origin.x &&
      box1.m_origin.y < box2.m_origin.y + box2.m_size.y && box1.m_origin.y + box2.m_origin.y > box2.m_origin.y &&
      box1.m_origin.z < box2.m_origin.z + box2.m_size.z && box1.m_origin.z + box2.m_origin.z > box2.m_origin.z;
  }

};