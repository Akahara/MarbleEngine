#pragma once

#include <string>

#include <glm/glm.hpp>

#include "Camera.h"

namespace Renderer {

    class Cubemap {
    public:
        Cubemap(const std::string& front, const std::string& back,
            const std::string& left, const std::string& right,
            const std::string& top, const std::string& bottom);
        ~Cubemap();

        void Bind() const;
        static void Unbind();

        Cubemap(const Cubemap&) = delete;
        Cubemap& operator=(const Cubemap&) = delete;
    private:
        unsigned int m_id;
    };


    // TODO move the renderer
    namespace CubemapRenderer {

        void Init();
        void DrawCubemap(const Cubemap& cubemap, const Camera& camera, const glm::vec3& offset = { 0,0,0 });
        void Shutdown();

    }

}