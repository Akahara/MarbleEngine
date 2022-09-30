#include "MeshGenerator.h"

using namespace Renderer;

namespace TerrainMeshGenerator {

Mesh generateMesh(const HeightMap &heightmap, glm::vec3 meshSize)
{
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;

  for (int y = 0; y < (int)heightmap.getMapHeight(); y++) {
    for (int x = 0; x < (int)heightmap.getMapWidth(); x++) {
      Vertex &vertex = vertices.emplace_back();
      vertex.position = { x * meshSize.x / heightmap.getMapWidth(), heightmap.getHeight(x, y) * meshSize.y, y * meshSize.z / heightmap.getMapHeight() };
      vertex.uv = { (float)x / heightmap.getMapWidth(), (float)y / heightmap.getMapHeight() };
      vertex.normal = { 0, 0, 0 };
    }
  }

  for (int x = 0; x < heightmap.getMapWidth()-1; x++) {
    for (int y = 0; y < heightmap.getMapHeight()-1; y++) {
      unsigned int a1 = y * heightmap.getMapWidth() + x;
      unsigned int a2 = y * heightmap.getMapWidth() + x + 1;
      unsigned int a3 = (y + 1) * heightmap.getMapWidth() + x;
      unsigned int a4 = (y + 1) * heightmap.getMapWidth() + x + 1;
      indices.push_back(a1);
      indices.push_back(a3);
      indices.push_back(a2);
      indices.push_back(a2);
      indices.push_back(a3);
      indices.push_back(a4);
    }
  }

  Mesh mesh{ vertices, indices };
  return mesh;
}


}