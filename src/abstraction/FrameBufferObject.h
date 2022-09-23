#pragma once
#include "BufferObject.h"

namespace Renderer {

class FrameBufferObject :  public BufferObject
	{
	public : 
		
		FrameBufferObject();
		~FrameBufferObject();

		void Bind()const;
		void Unbind()const;
		void Delete();

		unsigned int GetTextureColorBuffer() const { return m_TextureColorBuffer; }

	private : 

		unsigned int m_TextureColorBuffer;
		unsigned int m_RenderBuffer;

	};

}

