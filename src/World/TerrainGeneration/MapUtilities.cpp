#include "MapUtilities.h"

#include "../../Utils/Debug.h"

using namespace Renderer;

namespace MapUtilities {

Texture genTextureFromHeightmap(const HeightMap &heightmap, unsigned int minX, unsigned int minY, unsigned int width, unsigned int height)
{
  assert(minX < heightmap.getMapWidth() && minY < heightmap.getMapHeight());
  float *heights;
  bool areHeightExtracted = false;
  width = std::min(width, heightmap.getMapWidth() - minX);
  height = std::min(height, heightmap.getMapHeight() - minY);
  assert(width * height > 0);

  if (minX == 0 && minY == 0 && width == heightmap.getMapWidth() && height == heightmap.getMapHeight()) {
    heights = (float*)heightmap.getBackingArray();
  } else {
    heights = new float[width * height];
    areHeightExtracted = true;
    for (unsigned int x = 0; x < width; x++) {
      for (unsigned int y = 0; y < height; y++) {
        heights[x + y * width] = heightmap.getHeight(minX + x, minY + y);
      }
    }
  }

  Texture texture = Texture::createTextureFromData(heights, width, height, 1);

  if (areHeightExtracted)
    delete[] heights;

  return texture;
}

}