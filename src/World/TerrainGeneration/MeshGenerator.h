#pragma once

#include <glm/glm.hpp>

#include "HeightMap.h"
#include "../../abstraction/Mesh.h"

namespace TerrainMeshGenerator {

Renderer::Mesh generateMesh(const HeightMap &heightmap, glm::vec3 meshSize);

}
