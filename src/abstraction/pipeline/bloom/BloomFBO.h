#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "../../FrameBufferObject.h"
#include "../../Texture.h"
#include "../../Window.h"

#include <glad/glad.h>

struct BloomMip {

	glm::vec2 resolution;
	Renderer::Texture texture;
};


class BloomFBO {


private:
    std::vector<BloomMip*> m_mipChain; // switch to ptr
	Renderer::FrameBufferObject m_fbo;

public:





};