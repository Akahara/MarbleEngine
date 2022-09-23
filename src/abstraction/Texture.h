#pragma once
#include <string>

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



