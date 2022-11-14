#pragma once

#include <string>

namespace Utils {

inline bool replaceFirst(std::string &str, const std::string &replaced, const std::string &replacement)
{
  size_t foundAt = str.find(replaced);
  if (foundAt == std::string::npos)
    return false;
  str.replace(foundAt, replaced.length(), replacement);
  return false;
}

inline int replaceAll(std::string &str, const std::string &replaced, const std::string &replacement)
{
  int replacedCount = 0;
  while (replaceFirst(str, replaced, replacement))
    replacedCount++;
  return replacedCount;
}


}