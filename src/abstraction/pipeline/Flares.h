#pragma once

#include <vector>
#include <array>
#include <sstream>

#include <glm/glm.hpp>

#include "../Texture.h"
#include "../UnifiedRenderer.h"

struct Flare {
	Renderer::Texture texture;
	glm::vec2 screenPos;
	int texId;
};

class FlareRenderer {
private:
	const static int NUMBER_OF_FLARES = 10;
	const static int INDICES_COUNT = NUMBER_OF_FLARES * 6;

	struct QuadVertex {
		glm::vec3 Position;
		glm::vec2 TexCoords;
		float	  TexID;
	};

	Renderer::VertexArray m_VAO;
	Renderer::VertexBufferObject m_VBO;
	Renderer::IndexBufferObject m_IBO;

	Renderer::Shader m_shader = Renderer::loadShaderFromFiles(
		"res/shaders/flare.vs",
		"res/shaders/flare.fs");

	std::array<Renderer::Texture, 10> m_textureSlots;

	QuadVertex* m_quadBuffer = nullptr;
	QuadVertex* m_quadBufferPtr = nullptr;
	unsigned int m_indexCount = 0;
	unsigned int m_textureSlotIndex = 1;

public:
	FlareRenderer()
	{
		unsigned int indices[INDICES_COUNT];
		unsigned int offset = 0;

		for (int i = 0; i < INDICES_COUNT; i += 6) {
			indices[i + 0] = 0 + offset;
			indices[i + 1] = 1 + offset;
			indices[i + 2] = 2 + offset;

			indices[i + 3] = 2 + offset;
			indices[i + 4] = 3 + offset;
			indices[i + 5] = 0 + offset;

			offset += 4;
		}


		m_VBO = Renderer::VertexBufferObject(4 * NUMBER_OF_FLARES * sizeof(QuadVertex));
		Renderer::VertexBufferLayout layout;
		layout.push<float>(3);		// Position
		layout.push<float>(2);		// TexCoords
		layout.push<float>(1);		// TexID

		m_IBO = Renderer::IndexBufferObject(indices, INDICES_COUNT);
		m_VAO.addBuffer(m_VBO, layout, m_IBO);
		m_VAO.unbind();


		GLint samplers[12] = { 0,1,2,3,4,5,6,7,8,9,10,11 }; // TODO shader code
		m_shader.bind();
		m_shader.setUniform1iv("u_Textures", 12, samplers);
		m_shader.unbind();

		m_quadBuffer = new QuadVertex[NUMBER_OF_FLARES * 4];
	}

	~FlareRenderer()
	{
		delete[] m_quadBuffer;
	}

	void renderFlares(const std::vector<Flare>& flares, float brightness)
	{
		m_shader.bind();
		m_shader.setUniform1f("u_brightness", brightness);
		m_quadBufferPtr = m_quadBuffer;

		for (const auto& flare : flares) {

			// add flare to buffer

			glm::vec2 position = flare.screenPos;
			int textureIndex = flare.texId;
			float textureIndexAsFloat = (float)textureIndex; // avoids compilation warnings...
			glm::vec2 size = { 1,1 };

			// Bottom left
			m_quadBufferPtr->Position = { position.x, position.y, 0.0f };
			m_quadBufferPtr->TexCoords = { 0.0f, 0.0f };
			m_quadBufferPtr->TexID = textureIndexAsFloat;
			m_quadBufferPtr++;

			// Bottom right
			m_quadBufferPtr->Position = { position.x + size.x, position.y, 0.0f };
			m_quadBufferPtr->TexCoords = { 1.0f, 0.0f };
			m_quadBufferPtr->TexID = textureIndexAsFloat;
			m_quadBufferPtr++;

			//top right
			m_quadBufferPtr->Position = { position.x + size.x, position.y + size.y, 0.0f };
			m_quadBufferPtr->TexCoords = { 1.0f, 1.0f };
			m_quadBufferPtr->TexID = textureIndexAsFloat;
			m_quadBufferPtr++;

			// top left
			m_quadBufferPtr->Position = { position.x , position.y + size.y, 0.0f };
			m_quadBufferPtr->TexCoords = { 0.0f, 1.0f };
			m_quadBufferPtr->TexID = textureIndexAsFloat;
			m_quadBufferPtr++;

			m_indexCount += 6;

			flare.texture.bind(textureIndex);
		}

		GLsizeiptr size = (uint8_t*)m_quadBufferPtr - (uint8_t*)m_quadBuffer;

		m_VBO.bind();
		m_VAO.sendToGPU(size, m_quadBuffer);

		m_VAO.bind();

		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		m_indexCount = 0;
		m_textureSlotIndex = 1;
		m_VAO.unbind();
		m_shader.unbind();

	}
};

class FlareManager {
private:
	static constexpr float SPACING = .6f;
	std::vector<Flare> m_flares;

	FlareRenderer m_flareRenderer;

public:
	FlareManager()
	{

		// -- Main sun texture
		m_flares.push_back({
				Renderer::Texture{"res/textures/flare/sun.png"},
				{0,0},
				0
			});

		for (int i = 1; i < 10; i++) {
			std::stringstream ss;
			
			ss << "res/textures/flare/tex" << i << ".png";
			m_flares.push_back({
				Renderer::Texture{ss.str()},
				{0,0},
				i
			});
		}
	}

	glm::vec2 convertToScreenSpace(const glm::vec3& worldPos, const glm::mat4& VP)
	{
		glm::vec4 coords{ worldPos.x, worldPos.y, worldPos.z, 1.f };
		coords = VP * coords;
		if (coords.w <= 0) { return { -10,-10 }; }

		float x = (coords.x / (float)coords.w + 1.f) / 2.f;
		float y = 1.f - ((coords.y / (float)coords.w + 1.f) / 2.f);

		return { x, y };
	}

	void computeFlarePositions(const glm::vec2& sunToCenter, const glm::vec2& sunCoords)
	{
		m_flares[0].screenPos = sunCoords;
		for (unsigned int i = 1; i < m_flares.size(); i++) {
			glm::vec2 direction = sunToCenter;
			direction *= (float)i * SPACING;

			glm::vec2 textureSizeIn2D = m_flares[i].texture.getSize() / glm::vec2{float(Window::getWinWidth()), float(Window::getWinHeight())};
			glm::vec2 middleOfTexture = (textureSizeIn2D)/2.f;

			glm::vec2 flarePos = sunCoords - middleOfTexture + direction;


			m_flares[i].screenPos = glm::vec2{ (flarePos.x - 0.5f) * 2.F, (flarePos.y - 0.5f) * 2.F };
		}
	}

	// TODO batch render
	void render(const glm::vec3& sunWorldPos, const Renderer::Camera& camera) {

		glm::vec2 sunCoords = convertToScreenSpace(sunWorldPos, camera.getViewProjectionMatrix());

		glm::vec2 sunToCenter = glm::vec2{ 0.5f, 0.5f } - sunCoords;
		float length = 1.f - glm::length(sunToCenter);
		float brightness = (length / 0.6f);

		if (brightness <= 0) return;

		computeFlarePositions(sunToCenter, sunCoords);

		m_flareRenderer.renderFlares(m_flares, brightness);
	}
};


