#pragma once

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace Mathf {

static constexpr float PI = glm::pi<float>();

/* https://stackoverflow.com/questions/14997165/fastest-way-to-get-a-positive-modulo-in-c-c */
inline int positiveModulo(int i, int n) { return (i % n + n) % n; }

template<class T, class S>
inline T lerp(const T &a, const T &b, const S &x) { return a + (b - a) * x; }

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