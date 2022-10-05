#include "MeshGenerator.h"
#include <glm/geometric.hpp>
#include <iostream>

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


      if (x % 2 == 0) {
          // point en bas à gauche d'un quad
          // on veut donc calculer les vecteur A et B qui correspondent à p2-p1 et p3-p1

          // avec p2 = {p1.x, heightmap(x, y+1), p1.y + ystep }
          // avec p3 = {p1.x + xstep, heightmap(x+1, y), p1.y }

          // on fait ensuite A = p2-p1 = {0, heightmap(x, y+1) - heightmap(x,y), ystep}
          // on fait ensuite B = p3-p1 = {xstep, heightmap(x+1, y) - heightmap(x,y), 0}


          /*
          	Set Normal.x to (multiply U.y by V.z) minus (multiply U.z by V.y)
	        Set Normal.y to (multiply U.z by V.x) minus (multiply U.x by V.z)
	        Set Normal.z to (multiply U.x by V.y) minus (multiply U.y by V.x)

          
          1*/

          glm::vec3 A = {0 , heightmap.getHeight(x + 1, y) - heightmap.getHeight(x, y) , meshSize.z / heightmap.getMapHeight() };
          glm::vec3 B = { meshSize.x / heightmap.getMapWidth() , heightmap.getHeight(x + 1, y+1) - heightmap.getHeight(x, y), 0  };


          glm::vec3 N = {   (A.y * B.z) - (A.z * B.y),
                            (A.z * B.x) - (A.x * B.z),
                            (A.x * B.y) - (A.y * B.x) };

          vertex.normal = N;
      }
      else {
          // p1 = bas a droite, p2 = haut gauche, p3 = haut droite

          // avec p2 = {p1.x - xstep, heightmap(x-1, y+1), p1.y + ystep }
          // avec p3 = {p1.x , heightmap(x, y+1), p1.y + ystep }

          glm::vec3 A = {  -meshSize.x / heightmap.getMapWidth() , heightmap.getHeight(x - 1, y +1) - heightmap.getHeight(x, y), meshSize.z / heightmap.getMapHeight() };
          glm::vec3 B = { 0 , heightmap.getHeight(x, y + 1) - heightmap.getHeight(x, y), meshSize.z / heightmap.getMapHeight() };


          glm::vec3 N = { (A.y * B.z) - (A.z * B.y),
                            (A.z * B.x) - (A.x * B.z),
                            (A.x * B.y) - (A.y * B.x) };

          vertex.normal = N;

          //std::cout << vertex.normal.x << " " << vertex.normal.y << " "<<vertex.normal.z << std::endl;


      }

    }
  }

  for (int x = 0; x < (int)heightmap.getMapWidth() - 1; x++) {
    for (int y = 0; y < (int)heightmap.getMapHeight()-1; y++) {
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