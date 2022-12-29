#include "WaterSource.h"

WaterSource &WaterSource::operator=(const WaterSource &other)
{
  m_height = other.m_height;
  m_position = other.m_position;
  m_size = other.m_size;
  return *this;
}
