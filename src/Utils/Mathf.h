#pragma once

#include <iostream>
#include <limits>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace Mathf {

static constexpr float PI = glm::pi<float>();

inline int positiveModulo(int i, int n)
{
  return (i % n + n) % n;
}

inline unsigned int roundToNearestDivisor(unsigned int x, unsigned int d)
{
  return x / d * d;
}

template<class T, class S>
inline T lerp(const T &a, const T &b, const S &x)
{
  return a + (b - a) * x;
}

template<class T>
inline T mix(const T &x, const T &xMin, const T &xMax, const T &newMin, const T &newMax)
{
  return (x - xMin) / (xMax - xMin) * (newMax - newMin) + newMin;
}

inline float inverseLerp(float xx, float yy, float value)
{
  return (value - xx) / (yy - xx);
}

inline glm::vec3 unitVectorFromRotation(float yaw, float pitch)
{
  float cy = cos(yaw),   sy = sin(yaw);
  float cp = cos(pitch), sp = sin(pitch);
  return { -sy*cp, sp, -cy*cp };
}

inline float fract(float x)
{
  return x - floor(x);
}

inline float rand(float s)
{
  return fract(sin(s * 12.9898f) * 43758.5453f);
}

template<class T>
constexpr inline bool isPowerOfTwo(T x)
{
  return (x & (x - 1)) == 0;
}

constexpr inline unsigned int ceilToPowerOfTwo(unsigned int x)
{
  // https://stackoverflow.com/questions/466204/rounding-up-to-next-power-of-2
  x--;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  x++;
  return x;
}

namespace Detail {
// https://gist.github.com/alexshtf/eb5128b3e3e143187794
double constexpr sqrtNewtonRaphson(double x, double curr, double prev)
{
  return curr == prev ? curr : sqrtNewtonRaphson(x, 0.5 * (curr + x / curr), curr);
}
}

double constexpr sqrt(double x)
{
  return x >= 0 && x < std::numeric_limits<double>::infinity()
	? Detail::sqrtNewtonRaphson(x, x, 0)
	: std::numeric_limits<double>::quiet_NaN();
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