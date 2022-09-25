#include "Renderer.h"
#include "VertexArray.h"
#include "IndexBufferObject.h"
#include "VertexBufferObject.h"
#include "FrameBufferObject.h"
#include "Texture.h"
#include "TextureAtlas.h"
#include "Camera.h"

#include <array>
#include <stdlib.h>
#include <stdio.h>

void GLClearError()
{
	while (glGetError() != GL_NO_ERROR);
}

bool GLLogCall(const char* function, const char* file, int line) {

	while (GLenum error = glGetError()) {
		std::cout << "[OpenGL Error] (" << GLTranslateError(error) << "): " << function << " " << file << " " << "line" << std::endl;
		return false;
	}
	return true;
}

const char* GLTranslateError(GLenum error) {

	switch (error) {

		case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
		case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
		case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
		case GL_NO_ERROR: return "";
	}

}

void GLAPIENTRY openglMessageCallback(GLenum source, GLenum type, GLuint id,
	GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	std::cerr
		<< " type     = 0x" << std::hex << type << "\n"
		<< " severity = 0x" << std::hex << severity << "\n"
		<< " message  = " << message
		<< std::dec << std::endl;
}

//============================================================================================//

using namespace Renderer;

static const size_t MaxQuadCount = 1000;
static const size_t MaxVertexCount = MaxQuadCount * 4;
static const size_t MaxIndicesCount = MaxQuadCount * 6;
static const size_t MaxTextures = 32;

struct Vertex {

	glm::vec3 Position;
	glm::vec4 Color;
	glm::vec2 TexCoords;
	float	  TexID;

};

struct RendererData {


	std::shared_ptr<VertexArray>			QuadVA;
	std::shared_ptr<VertexBufferObject>		QuadVBO;
	std::shared_ptr<IndexBufferObject>		QuadIBO;

	std::shared_ptr<Shader>					QuadShader;

	std::shared_ptr<Texture>		WhiteTexture;

	unsigned int IndexCount = 0;

	Vertex* QuadBuffer = nullptr;
	Vertex* QuadBufferPtr = nullptr;

	std::array<std::shared_ptr<Texture>, MaxTextures> TextureSlots;
	unsigned int TextureSlotIndex = 1;

	bool isBatching = false;


};

static RendererData s_RendererData;

namespace Renderer {

	void Renderer::Clear(const float& r) {
		glClearColor(r, 0.13f, 0.17f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void Renderer::Draw(const VertexArray& va, const IndexBufferObject& ibo, const Shader& shader) {
		shader.Bind();
		va.Bind();
		ibo.Bind();
		std::cout << ibo.getCount() << std::endl;

		glDrawElements(GL_TRIANGLES, ibo.getCount(), GL_UNSIGNED_INT, nullptr);
	}

	void Renderer::Init(){

		std::string vs = R"glsl(
			#version 330 core

			layout(location = 0) in vec3 position;
			layout(location = 1) in vec4 aColor;
			layout(location = 2) in vec2 texCoord;
			layout (location = 3) in float a_TexIndex;

			out vec2 v_TexCoord;
			out float v_TexIndex;
			out vec4 v_Color;


			uniform mat4 u_MVP;

			void main(){
	
				v_TexIndex = a_TexIndex;
				gl_Position = u_MVP * vec4(position,1.0f);
				v_TexCoord = texCoord;
				v_Color = aColor;
			};
		)glsl";

		std::string fs = R"glsl(
			#version 330 core

			layout(location = 0) out vec4 color;

			in vec2 v_TexCoord;
			in float v_TexIndex;
			in vec4 v_Color;

			uniform sampler2D u_Textures[8];

			void main()
			{
				int index = int(v_TexIndex);
				color = texture(u_Textures[index], v_TexCoord) * v_Color;
				//float sample = (color.x+color.y+color.z)/3;
				//color = vec4(sample, sample, sample, 1);
				color.rgb = vec3(color.r); // the perlin noise texture contains a red channel only !
				color.a = 1;
			};
		)glsl";
		

		/*

			std::string fs = R"glsl(
			#version 330 core

			float lensSize = 0.4f; // 0.4
			vec2 resolution = vec2(800.0f, 800.0f);

			layout(location = 0) out vec4 outColor;

			in vec2 v_TexCoord;
			in float v_TexIndex;
			in vec4 v_Color;

			


			uniform sampler2D u_Textures[8];
			vec4 Vertex_UV;
			const float PI = 3.1415926535;

			void main(){
					int index = int(v_TexIndex);
					float aperture = 178.0;
					float apertureHalf = 0.5 * aperture * (PI / 180.0);
					float maxFactor = sin(apertureHalf);

					vec2 uv;
				  vec2 xy = 2.0 * v_TexCoord.xy - 1.0;
				  float d = length(xy);
				  if (d < (2.0-maxFactor))
				  {
					d = length(xy * maxFactor);
					float z = sqrt(1.0 - d * d);
					float r = atan(d, z) / PI;
					float phi = atan(xy.y, xy.x);
    
					uv.x = r * cos(phi) + 0.5;
					uv.y = r * sin(phi) + 0.5;
				  }
				  else
				  {
					uv = v_TexCoord.xy;
				  }
				  vec4 c = texture2D(u_Textures[index], uv) * v_Color;
				  outColor = c;
			};
		)glsl";*/

		s_RendererData.QuadShader = std::make_shared<Shader>(vs, fs);
		s_RendererData.QuadShader->Bind();

		s_RendererData.QuadBuffer = new Vertex[MaxVertexCount];

		unsigned int indices[MaxIndicesCount];
		unsigned int offset = 0;

		for (int i = 0; i < MaxIndicesCount; i += 6) {

			indices[i + 0] = 0 + offset;
			indices[i + 1] = 1 + offset;
			indices[i + 2] = 2 + offset;

			indices[i + 3] = 2 + offset;
			indices[i + 4] = 3 + offset;
			indices[i + 5] = 0 + offset;

			offset += 4;
		}
		
		s_RendererData.QuadVA = std::make_shared<VertexArray>();
		s_RendererData.QuadVBO = std::make_shared<VertexBufferObject>(MaxVertexCount * sizeof(Vertex));



		VertexBufferLayout layout;
		layout.push<float>(3);		// Position
		layout.push<float>(4);		// Color
		layout.push<float>(2);		// TexCoords
		layout.push<float>(1);		// TexID

		s_RendererData.QuadVA->addBuffer(*s_RendererData.QuadVBO, layout);
		s_RendererData.QuadIBO = std::make_shared<IndexBufferObject>(indices, MaxIndicesCount);
		

		s_RendererData.WhiteTexture = std::make_shared<Texture>("whitePixel.png");
		s_RendererData.TextureSlots[0] = s_RendererData.WhiteTexture;


		GLint samplers[8] = { 0,1,2,3,4,5,6,7 };
		s_RendererData.QuadShader->SetUniform1iv("u_Textures", 8, samplers);
		
	}

	void Renderer::Shutdown() {

		delete[] s_RendererData.QuadBuffer;
	}

	void Renderer::BeginBatch(const Camera& camera){

		s_RendererData.QuadShader->Bind();
		s_RendererData.QuadShader->SetUniformMat4f("u_MVP", camera.getViewProjectionMatrix());
		s_RendererData.QuadBufferPtr = s_RendererData.QuadBuffer;

		s_RendererData.isBatching = true;

	}
	void Renderer::EndBatch(){


		GLsizeiptr size = (uint8_t*)s_RendererData.QuadBufferPtr - (uint8_t*)s_RendererData.QuadBuffer;

		s_RendererData.QuadVBO->Bind();
		s_RendererData.QuadVA->SendToGPU(size, s_RendererData.QuadBuffer);


	}


	void Renderer::Flush(){


		s_RendererData.isBatching = false;

		
		for (unsigned int i = 0; i < s_RendererData.TextureSlotIndex; i++) {

			s_RendererData.TextureSlots[i]->Bind(i);
		}
		
		s_RendererData.QuadVA->Bind();
		s_RendererData.QuadIBO->Bind();
		GLCall(glDrawElements(GL_TRIANGLES, s_RendererData.IndexCount, GL_UNSIGNED_INT, nullptr));

		s_RendererData.IndexCount = 0;
		s_RendererData.TextureSlotIndex = 1;
		s_RendererData.QuadVA->Unbind();


	}

	void Renderer::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& colors) {


		if (s_RendererData.IndexCount >= MaxIndicesCount) { // TODO query

			EndBatch();
			Flush();
			s_RendererData.QuadBufferPtr = s_RendererData.QuadBuffer;

		}

		float textureIndex = 0.0f;

		// Bottom left
		s_RendererData.QuadBufferPtr->Position = { position.x, position.y, 0.0f }; // 2D only
		s_RendererData.QuadBufferPtr->Color = colors;
		s_RendererData.QuadBufferPtr->TexCoords = { 0.0f, 0.0f };
		s_RendererData.QuadBufferPtr->TexID = textureIndex;
		s_RendererData.QuadBufferPtr++;

		// Bottom right
		s_RendererData.QuadBufferPtr->Position = { position.x + size.x, position.y, 0.0f }; // 2D only
		s_RendererData.QuadBufferPtr->Color = colors;
		s_RendererData.QuadBufferPtr->TexCoords = { 0.0f, 0.0f };
		s_RendererData.QuadBufferPtr->TexID = textureIndex;
		s_RendererData.QuadBufferPtr++;

		//top right
		s_RendererData.QuadBufferPtr->Position = { position.x + size.x, position.y + size.y, 0.0f }; // 2D only
		s_RendererData.QuadBufferPtr->Color = colors;
		s_RendererData.QuadBufferPtr->TexCoords = { 0.0f, 0.0f };
		s_RendererData.QuadBufferPtr->TexID = textureIndex;
		s_RendererData.QuadBufferPtr++;

		// top left
		s_RendererData.QuadBufferPtr->Position = { position.x , position.y + size.y, 0.0f }; // 2D only
		s_RendererData.QuadBufferPtr->Color = colors;
		s_RendererData.QuadBufferPtr->TexCoords = { 0.0f, 0.0f };
		s_RendererData.QuadBufferPtr->TexID = textureIndex;
		s_RendererData.QuadBufferPtr++;

		s_RendererData.IndexCount += 6;



	}

	void Renderer::DrawQuad(const glm::vec3& position, const glm::vec2& size, const std::shared_ptr<Texture>& texture, const glm::vec4& colors) {

		if (s_RendererData.IndexCount >= MaxIndicesCount || s_RendererData.TextureSlotIndex > 7) { // TODO query

			EndBatch();
			Flush();
			s_RendererData.QuadBufferPtr = s_RendererData.QuadBuffer;

		}

		float textureIndex = 0.0f;
		
		for (unsigned int i = 1; i < s_RendererData.TextureSlotIndex; i++) {


			if (s_RendererData.TextureSlots[i]->getId() == texture->getId()) {
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f) {
			textureIndex = (float)s_RendererData.TextureSlotIndex;
			s_RendererData.TextureSlots[s_RendererData.TextureSlotIndex] = texture;
			s_RendererData.TextureSlotIndex++;
		}
				


		// Bottom left
		s_RendererData.QuadBufferPtr->Position = { position.x, position.y, 0.0f }; 
		s_RendererData.QuadBufferPtr->Color = colors;
		s_RendererData.QuadBufferPtr->TexCoords = { 0.0f, 0.0f };
		s_RendererData.QuadBufferPtr->TexID = textureIndex;
		s_RendererData.QuadBufferPtr++;

		// Bottom right
		s_RendererData.QuadBufferPtr->Position = { position.x + size.x, position.y, 0.0f }; 
		s_RendererData.QuadBufferPtr->Color = colors;
		s_RendererData.QuadBufferPtr->TexCoords = { 1.0f, 0.0f };
		s_RendererData.QuadBufferPtr->TexID = textureIndex;
		s_RendererData.QuadBufferPtr++;

		//top right
		s_RendererData.QuadBufferPtr->Position = { position.x + size.x, position.y + size.y, 0.0f }; 
		s_RendererData.QuadBufferPtr->Color = colors;
		s_RendererData.QuadBufferPtr->TexCoords = { 1.0f, 1.0f };
		s_RendererData.QuadBufferPtr->TexID = textureIndex;
		s_RendererData.QuadBufferPtr++;

		// top left
		s_RendererData.QuadBufferPtr->Position = { position.x , position.y + size.y, 0.0f };
		s_RendererData.QuadBufferPtr->Color = colors;
		s_RendererData.QuadBufferPtr->TexCoords = { 0.0f, 1.0f };
		s_RendererData.QuadBufferPtr->TexID = textureIndex;
		s_RendererData.QuadBufferPtr++;

		s_RendererData.IndexCount += 6;



	}

	void Renderer::DrawQuadFromAtlas(const glm::vec3& position, const glm::vec2& size, const TextureAtlas& textureAtlas, int x, int y, const glm::vec4& colors /*= {1.0f,1.0f,1.0f,1.0f}*/) {
		
		std::shared_ptr<Texture> texture = textureAtlas.getTexture();

		glm::vec2 textureCoords[] = {

			{ (x * textureAtlas.getSpriteWidth()) / textureAtlas.getSheetWidth(), (y * textureAtlas.getSpriteHeight()) / textureAtlas.getSheetHeight()},
			{ ((x + 1) * textureAtlas.getSpriteWidth()) / textureAtlas.getSheetWidth(), (y * textureAtlas.getSpriteHeight()) / textureAtlas.getSheetHeight()},
			{ ((x + 1) * textureAtlas.getSpriteWidth()) / textureAtlas.getSheetWidth(), ((y + 1) * textureAtlas.getSpriteHeight()) / textureAtlas.getSheetHeight()},
			{ (x * textureAtlas.getSpriteWidth()) / textureAtlas.getSheetWidth(), ((y + 1) * textureAtlas.getSpriteHeight()) / textureAtlas.getSheetHeight()}

		};

		if (s_RendererData.IndexCount >= MaxIndicesCount || s_RendererData.TextureSlotIndex > 7) { // TODO query

			EndBatch();
			Flush();
			s_RendererData.QuadBufferPtr = s_RendererData.QuadBuffer;

		}

		float textureIndex = 0.0f;

		for (unsigned int i = 1; i < s_RendererData.TextureSlotIndex; i++) {


			if (s_RendererData.TextureSlots[i]->getId() == texture->getId()) {
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f) {
			textureIndex = (float)s_RendererData.TextureSlotIndex;
			s_RendererData.TextureSlots[s_RendererData.TextureSlotIndex] = texture;
			s_RendererData.TextureSlotIndex++;
		}



		// Bottom left
		s_RendererData.QuadBufferPtr->Position = { position.x, position.y, 0.0f };
		s_RendererData.QuadBufferPtr->Color = colors;
		s_RendererData.QuadBufferPtr->TexCoords = textureCoords[0];
		s_RendererData.QuadBufferPtr->TexID = textureIndex;
		s_RendererData.QuadBufferPtr++;

		// Bottom right
		s_RendererData.QuadBufferPtr->Position = { position.x + size.x, position.y, 0.0f };
		s_RendererData.QuadBufferPtr->Color = colors;
		s_RendererData.QuadBufferPtr->TexCoords = textureCoords[1];
		s_RendererData.QuadBufferPtr->TexID = textureIndex;
		s_RendererData.QuadBufferPtr++;

		//top right
		s_RendererData.QuadBufferPtr->Position = { position.x + size.x, position.y + size.y, 0.0f };
		s_RendererData.QuadBufferPtr->Color = colors;
		s_RendererData.QuadBufferPtr->TexCoords = textureCoords[2];
		s_RendererData.QuadBufferPtr->TexID = textureIndex;
		s_RendererData.QuadBufferPtr++;

		// top left
		s_RendererData.QuadBufferPtr->Position = { position.x , position.y + size.y, 0.0f };
		s_RendererData.QuadBufferPtr->Color = colors;
		s_RendererData.QuadBufferPtr->TexCoords = textureCoords[3];
		s_RendererData.QuadBufferPtr->TexID = textureIndex;
		s_RendererData.QuadBufferPtr++;

		s_RendererData.IndexCount += 6;




	}

	
}