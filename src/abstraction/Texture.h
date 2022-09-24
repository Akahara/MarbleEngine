#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>

#include "stb_image.h"
#include "stb_image_write.h"


#define ASSERT(x) if (!(x)) __debugbreak();

#define GLCall(x) GLClearError();\
	x;\
	ASSERT(GLLogCall(#x, __FILE__, __LINE__))


void GLClearError();
bool GLLogCall(const char* function, const char* file, int line);
const char* GLTranslateError(GLenum error);
void GLAPIENTRY openglMessageCallback(GLenum source, GLenum type, GLuint id,
	GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

namespace Renderer {


	class Texture
	{

	private:
		unsigned int m_RendererID;
		std::string m_FilePath;
		unsigned char* m_LocalBuffer;
		int m_Width, m_Height, m_BPP;
	public:
		Texture(const std::string& path);
		Texture(unsigned int id) : m_RendererID(id) {}
		~Texture();

		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;

		void Bind(unsigned int slot = 0) const;
		void Unbind() const;

		void ChangeColor(uint32_t color);


		inline int GetWidth() const { return m_Width; }
		inline int GetHeight() const { return m_Height; }
		inline unsigned int getId() const  {
			return m_RendererID;
		}


	};	
}



