#pragma once

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace Mathf {

static constexpr float PI = glm::pi<float>();

inline int positiveModulo(int i, int n) { return (i % n + n) % n; }

template<class T, class S>
inline T lerp(const T &a, const T &b, const S &x) { return a + (b - a) * x; }

inline glm::vec3 unitVectorFromRotation(float yaw, float pitch)
{
  float cy = cos(yaw),   sy = sin(yaw);
  float cp = cos(pitch), sp = sin(pitch);
  return { -sy*cp, sp, -cy*cp };
}

inline float fract(float x)
{
  return x - (int)x;
}

inline float rand(float s)
{
  return fract(sin(s * 12.9898f) * 43758.5453f);
}

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