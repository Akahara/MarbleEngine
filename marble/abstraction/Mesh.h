#pragma once

#include <vector>
#include <array>

#include <glm/glm.hpp>

#include "VertexArray.h"
#include "Texture.h"
#include "Shader.h"
#include "../Utils/AABB.h"
#include "../Utils/Transform.h"

namespace Renderer {

struct BaseVertex {
  glm::vec3 position{ 0,0,0 };
  glm::vec2 uv{ 0,0 };
  glm::vec3 normal{ 0,1,0 };
  glm::vec3 color{ 1 };
  float texId = 0;

  static const VertexBufferLayout &getVertexBufferLayout()
  {
    static VertexBufferLayout layout = []() {
      VertexBufferLayout l;
      l.push<float>(3); // position
      l.push<float>(2); // uv
      l.push<float>(3); // normal
      l.push<float>(3); // color
      l.push<float>(1); // texId
      return l;
    }();
    return layout;
  }
};

struct BaseInstance {
  glm::vec3 position{0,0,0};
  glm::vec3 scale{1};
  // TODO add rotation to BaseInstance

  static const VertexBufferLayout &getVertexBufferLayout()
  {
    static VertexBufferLayout layout = []() {
      VertexBufferLayout l;
      l.push<float>(3); // position
      l.push<float>(3); // scale
      return l;
    }();
    return layout;
  }
};

/**
* A mesh contains references to a VAO and its components, textures and a bounding box.
* 
* The vertices used to generate the mesh are not kept in cpu memory.
* 
* Mesh can be considered as "renderable primitives", a final user should not have to
* use GL functions when using meshes.
* 
* Models can be generated using Renderer#loadMeshFromFile.
*/
class Model {
private:
  VertexBufferObject m_VBO;
  IndexBufferObject  m_IBO;
  unsigned int       m_verticesCount;
  AABB               m_boudingBox;

public:
  Model() : m_verticesCount(0) {}
  Model(const std::vector<BaseVertex> &vertices, const std::vector<unsigned int> &indices);
  Model(Model &&moved) noexcept;
  Model &operator=(Model &&moved) noexcept;
  Model &operator=(const Model&) = delete;
  Model(const Model&) = delete;

  unsigned int getVertexCount() const { return m_verticesCount; }
  const VertexBufferObject &getVBO() const { return m_VBO; }
  const IndexBufferObject &getIBO() const { return m_IBO; }
  const AABB &getBoundingBox() const { return m_boudingBox; }
  AABB getBoundingBoxInstance(glm::vec3 instancePosition, glm::vec3 instanceSize) const;
};

struct Material
{
  static constexpr unsigned int TEXTURE_SLOT_COUNT = 8;

  std::shared_ptr<Shader> shader;
  std::array<std::shared_ptr<Texture>, TEXTURE_SLOT_COUNT> textures;
};

class Mesh {
private:
  std::shared_ptr<Model>    m_model;
  std::shared_ptr<Material> m_material;
  Transform                 m_transform;
  VertexArray               m_VAO;

public:
  Mesh() {}
  Mesh(const std::shared_ptr<Model> &model, const std::shared_ptr<Material> &material);
  Mesh(Mesh &&moved) noexcept;
  Mesh &operator=(Mesh &&moved) noexcept;
  Mesh(Mesh&) = delete;
  Mesh &operator=(Mesh&) = delete;

  const std::shared_ptr<Model> &getModel() const { return m_model; }
  const std::shared_ptr<Material> &getMaterial() const { return m_material; }
  std::shared_ptr<Material> &getMaterial() { return m_material; }
  void setMaterial(const std::shared_ptr<Material> &material) { m_material = material; }
  const Transform &getTransform() const { return m_transform; }
  Transform &getTransform() { return m_transform; }
  void setTransform(const Transform &transform) { m_transform = transform; }
  const VertexArray &getVAO() const { return m_VAO; }
};

class InstancedMesh {
private:
  std::shared_ptr<Model>    m_model;
  std::shared_ptr<Material> m_material;
  VertexArray               m_VAO;
  VertexBufferObject        m_instanceBuffer;

  static constexpr size_t m_instanceSize = sizeof(BaseInstance);

public:
  InstancedMesh() {}
  InstancedMesh(const std::shared_ptr<Model> &model, const std::shared_ptr<Material> &material, size_t instanceCount, const BaseInstance *instances=nullptr);
  InstancedMesh(InstancedMesh &&moved) noexcept;
  InstancedMesh &operator=(InstancedMesh &&moved) noexcept;
  InstancedMesh(const InstancedMesh &) = delete;
  InstancedMesh &operator=(const InstancedMesh &) = delete;

  const std::shared_ptr<Model> &getModel() const { return m_model; }
  const std::shared_ptr<Material> &getMaterial() const { return m_material; }
  std::shared_ptr<Material> &getMaterial() { return m_material; }
  void setMaterial(const std::shared_ptr<Material> &material) { m_material = material; }
  const VertexArray &getVAO() const { return m_VAO; }
  size_t getInstanceCount() const { return m_instanceBuffer.getSize() / m_instanceSize; }

  void updateInstances(const BaseInstance *instances, size_t beginInstance, size_t endInstance); // updates the instances at indices [begin,end[
  void replaceInstances(const BaseInstance *instances, size_t instanceCount);
};

struct TerrainRegion {
  float minX, minY, maxX, maxY;
};

/*
 * Heightmaps are simply functions taking inputs in the xz plane and returning a height value.
 * By convention, heightmap parameters are called x and y instead of x and z.
 * Heightmaps are tipically not required to be sampleable in the whole xz plane, methods needing
 * an heightmap should precise in what ranges the heightmap must be sampleable.
 * Heigtmaphs must be predictible, that is, two calls with the same xy values must always produce
 * the same output.
 */
template<typename T>
concept Heightmap = requires(T heightmap, float x, float y)
{
  { heightmap(x, y) } -> std::convertible_to<float>;
};

class TerrainMesh {
public:
  static constexpr int CHUNK_SIZE = 50;
  struct Chunk {
    VertexArray        vao;
    VertexBufferObject vbo;
    glm::ivec2         position;
    AABB               worldBoundingBox;
  };

private:
  std::vector<Chunk>        m_chunks;
  std::shared_ptr<Material> m_material;
  Transform                 m_transform;
  IndexBufferObject         m_ibo; // a single ibo is enough for all chunks

public:
  TerrainMesh() : TerrainMesh(nullptr) {}
  TerrainMesh(const std::shared_ptr<Material> &material);
  TerrainMesh(const TerrainMesh &) = delete;
  TerrainMesh &operator=(const TerrainMesh &) = delete;
  //TerrainMesh(TerrainMesh &&);
  //TerrainMesh &operator=(TerrainMesh &&);

  /*
   * Rebuild the mesh to cover the given region.
   *
   * The generated chunks may cover more than the given region, they tipically
   * cover multiples of CHUNK_SIZE world units.
   *
   * The heightmap must be sampleable in the given region plus a small
   * margin, samples outside are used to compute terrain normals.
   */
  template<Heightmap Heightmap>
  void rebuildMesh(Heightmap heightmap, TerrainRegion region);

  Transform &getTransform() { return m_transform; }
  const Transform &getTransform() const { return m_transform; }
  void setTransform(const Transform &transform) { m_transform = transform; }
  const std::vector<Chunk> &getChunks() const { return m_chunks; }
  const IndexBufferObject &getIBO() const { return m_ibo; }
  const std::shared_ptr<Material> &getMaterial() const { return m_material; }
  std::shared_ptr<Material> &getMaterial() { return m_material; }
  void setMaterial(const std::shared_ptr<Material> &material) { assert(material != nullptr); m_material = material; }

private:
  template<Heightmap Heightmap>
  Chunk generateChunk(Heightmap &heightmap, glm::ivec2 chunkPosition);
};

class NormalsMesh {
private:
  VertexBufferObject m_VBO;
  IndexBufferObject  m_IBO;
  VertexArray        m_VAO;
  unsigned int       m_verticesCount;

public:
  NormalsMesh();
  NormalsMesh(const std::vector<BaseVertex> &vertices);
  ~NormalsMesh();
  NormalsMesh(NormalsMesh &&moved) noexcept;
  NormalsMesh &operator=(NormalsMesh &&moved) noexcept;
  NormalsMesh &operator=(const NormalsMesh &) = delete;
  NormalsMesh(const NormalsMesh &) = delete;

  unsigned int getVertexCount() const { return m_verticesCount; }

  void draw() const;
};

}
