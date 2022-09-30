#pragma once

#include "HeightMap.h"
#include "../../abstraction/Texture.h"

namespace MapUtilities {

Renderer::Texture genTextureFromHeightmap(const HeightMap &heightmap, unsigned int minX=0, unsigned int minY=0, unsigned int width=-1, unsigned int height=-1);

}