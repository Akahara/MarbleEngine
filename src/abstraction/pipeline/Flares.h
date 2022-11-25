#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <array>
#include <sstream>
#include "../Texture.h"

#include "../UnifiedRenderer.h"

struct Flare {

	Renderer::Texture texture;
	glm::vec2 screenPos;
	int texId;
};

class FlareRenderer {

private :

	const static int NUMBER_OF_FLARES = 9;
	const static int INDICES_COUNT = 9 * 6;

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

	std::array<Renderer::Texture, 9> m_textureSlots;


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

	~FlareRenderer() {
		delete[] m_quadBuffer;
	}

	void renderFlares(const std::vector<Flare>& flares)
	{
		m_shader.bind();
		m_quadBufferPtr = m_quadBuffer;

		for (const auto& flare : flares) {

			// add flare to buffer

			glm::vec2 position = flare.screenPos;
			float textureIndex = flare.texId;
			glm::vec2 size = { 1,1 };

			// Bottom left
			m_quadBufferPtr->Position = { position.x, position.y, 0.0f };
			m_quadBufferPtr->TexCoords = { 0.0f, 0.0f };
			m_quadBufferPtr->TexID = textureIndex;
			m_quadBufferPtr++;

			// Bottom right
			m_quadBufferPtr->Position = { position.x + size.x, position.y, 0.0f };
			m_quadBufferPtr->TexCoords = { 1.0f, 0.0f };
			m_quadBufferPtr->TexID = textureIndex;
			m_quadBufferPtr++;

			//top right
			m_quadBufferPtr->Position = { position.x + size.x, position.y + size.y, 0.0f };
			m_quadBufferPtr->TexCoords = { 1.0f, 1.0f };
			m_quadBufferPtr->TexID = textureIndex;
			m_quadBufferPtr++;

			// top left
			m_quadBufferPtr->Position = { position.x , position.y + size.y, 0.0f };
			m_quadBufferPtr->TexCoords = { 0.0f, 1.0f };
			m_quadBufferPtr->TexID = textureIndex;
			m_quadBufferPtr++;

			m_indexCount += 6;


		}
		
		GLsizeiptr size = (uint8_t*)m_quadBufferPtr - (uint8_t*)m_quadBuffer;

		m_VBO.bind();
		m_VAO.sendToGPU(size, m_quadBuffer);

		for (unsigned int i = 0; i < 9; i++) {

				m_textureSlots[i].bind(i);
		}

		m_VAO.bind();
		glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);

		m_indexCount = 0;
		m_textureSlotIndex = 1;
		m_VAO.unbind();

	}


};
class FlareManager {

private:

	std::vector<Flare> m_flares;
	float spacing = 0.6f;

	FlareRenderer m_flareRenderer;

public:

	FlareManager() 
	{
		for (int i = 1; i < 10; i++) {


			std::stringstream ss;
			ss << "res/textures/flare/tex" << i << ".png";
			m_flares.push_back(
				{
				Renderer::Texture{ss.str()},
				{0,0}, i
				}
			);

		}


	}

	void computeFlarePositions(const glm::vec2& screenSunPos) {

		const glm::vec2 CENTER = { 0.5f, 0.5f };

		glm::vec2 sunToCenterDirection = CENTER - screenSunPos;

		for (unsigned i = 0; i < m_flares.size(); i++)
		{
			m_flares[i].screenPos = CENTER; //+ sunToCenterDirection * float(i) * spacing;
		}
	}

	// TODO batch render
	void render(const glm::vec2& screenSunPos, const Renderer::Camera& camera) {

		computeFlarePositions(screenSunPos);		
		m_flareRenderer.renderFlares(m_flares);
	}


};


