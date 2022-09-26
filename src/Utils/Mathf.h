#pragma once

#include <iostream>

#include <glm/glm.hpp>

namespace Mathf {

static constexpr float PI = glm::pi<float>();

}

template<typename T, glm::qualifier Q>
std::ostream &operator<<(std::ostream &out, const glm::vec<2, T, Q> &v)
{
  return out << "(" << v.x << ", " << v.y << ")";
}

template<typename T, glm::qualifier Q>
std::ostream &operator<<(std::ostream &out, const glm::vec<3, T, Q> &v)
{
  return out << "(" << v.x << ", " << v.y << ", " << v.z << ")";
}

template<typename T, glm::qualifier Q>
std::ostream &operator<<(std::ostream &out, const glm::vec<4, T, Q> &v)
{
  return out << "(" << v.x << ", " << v.y << ", " << v.z << "," << v.w << ")";
}