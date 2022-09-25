#pragma once
#include <string>

namespace Renderer {


	class Texture
	{

	private:
		unsigned int   m_RendererID;
		std::string    m_FilePath;
		int m_Width, m_Height, m_BPP;
	public:
	    Texture();
		Texture(const std::string& path);
		~Texture();
		Texture(Texture &&moved) noexcept;
		Texture &operator=(Texture &&moved) noexcept;
		Texture& operator=(const Texture&) = delete;
		Texture(const Texture&) = delete;

		void Bind(unsigned int slot = 0) const;
		void Unbind() const;
		void Delete();

		void ChangeColor(uint32_t color);

		inline int GetWidth() const { return m_Width; }
		inline int GetHeight() const { return m_Height; }
		inline unsigned int getId() const { return m_RendererID; }
	};	
}



