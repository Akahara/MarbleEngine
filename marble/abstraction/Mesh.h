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
