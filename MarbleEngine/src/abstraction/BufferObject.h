#pragma once

namespace Renderer {

class BufferObject {
protected:
  unsigned int m_renderID;

public:
  virtual void bind() const = 0;
  virtual void unbind() const = 0;
  virtual void destroy() = 0;

  BufferObject() : m_renderID(0) {}
  BufferObject(const BufferObject &) = delete;
  BufferObject &operator=(const BufferObject &) = delete;

  // Unsafe
  unsigned int getId() { return m_renderID; }
};

}
