#pragma once

#include <iostream>
#include<glad/glad.h>

#include "Texture.h"
#include "TextureAtlas.h"


#include "Shader.h"
#include "IndexBufferObject.h"
#include "VertexArray.h"
#include "FrameBufferObject.h"
#include "Camera.h"



namespace Renderer {

	class Renderer {


	public:

		static void Clear(const float& r);
		static void Draw(const VertexArray& va, const IndexBufferObject& ibo, const Shader& shader);

		static void Init();
		static void Shutdown();
		static void BeginBatch(const Camera& camera);
		static void EndBatch();
		static void Flush();

		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& colors);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const std::shared_ptr<Texture>& texture, const glm::vec4& colors = { 1.0f, 1.0f, 1.0f, 1.0f });
		static void DrawQuadFromAtlas(const glm::vec3& position, const glm::vec2& size, const TextureAtlas& texture, int x, int y, const glm::vec4& colors={ 1.0f,1.0f,1.0f,1.0f });
	};

}

#define ASSERT(x) if (!(x)) __debugbreak();

#define GLCall(x) GLClearError();\
	x;\
	ASSERT(GLLogCall(#x, __FILE__, __LINE__))


void GLClearError();
bool GLLogCall(const char* function, const char* file, int line);
const char* GLTranslateError(GLenum error);
void GLAPIENTRY openglMessageCallback(GLenum source, GLenum type, GLuint id,
	GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

