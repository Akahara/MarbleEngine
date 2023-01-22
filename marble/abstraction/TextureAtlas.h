#pragma once

#include <memory>
#include <string>

#include "Texture.h"

namespace Renderer {

/**
* Contains a set of textures wrapped in a single one, individual textures can
* be accessed using UV coordinates.
* Search the web for a more detailed explanation, it corresponds to the standard
* definition of "texture atlas".
*/
class TextureAtlas
{
private:
  std::shared_ptr<Texture> m_atlas;
  float m_sheetWidth,  m_sheetHeight;
  float m_spriteWidth, m_spriteHeight;
public:
	TextureAtlas(const std::string& path, float width, float height, float spriteWidth, float spriteHeight)
		: m_atlas(std::make_shared<Texture>(path)),
		m_sheetWidth(width),
		m_sheetHeight(height),
		m_spriteWidth(spriteWidth),
		m_spriteHeight(spriteHeight)
	{ }
		
	TextureAtlas(std::shared_ptr<Texture> texture, float width, float height, float spriteWidth, float spriteHeight)
		: m_atlas(texture),
		m_sheetWidth(width),
		m_sheetHeight(height),
		m_spriteWidth(spriteWidth),
		m_spriteHeight(spriteHeight)		
	{ }

	TextureAtlas(const TextureAtlas&) = delete;
	~TextureAtlas() {}

	inline std::shared_ptr<Texture> getTexture() const { return m_atlas; }
	inline float getSheetWidth()const { return m_sheetWidth; }
	inline float getSheetHeight() const { return m_sheetHeight; }
	inline float getSpriteWidth()const { return m_spriteWidth; }
	inline float getSpriteHeight() const { return m_spriteHeight; }
};

}

