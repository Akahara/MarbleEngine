#pragma once

#include "VertexArray.h"
#include "IndexBufferObject.h"
#include "VertexBufferObject.h"
#include "VertexBufferLayout.h"
#include "Shader.h"
#include <vector>


class Mesh
{

private:

	struct Vertex {

		glm::vec3 position;
		glm::vec2 uv;

	};

	Renderer::IndexBufferObject m_IBO;
	Renderer::VertexArray m_VAO;
	Renderer::VertexBufferObject m_VBO;

    std::vector<Vertex> m_Verticices;
    std::vector<unsigned int> m_Indicies;

    Texture m_Texture;


public:


	Mesh(const std::vector<Vertex>& verticices, const std::vector<unsigned int>& indicies) {



        m_VBO = Renderer::VertexBufferObject(verticices.data(), sizeof(Vertex) * verticices.size());
        m_IBO = Renderer::IndexBufferObject(indicies.data(), indicies.size());

        Renderer::VertexBufferLayout layout;
        layout.push<float>(3);
        layout.push<float>(2);
        m_VAO.Bind();
        m_VAO.addBuffer(m_VBO, layout);
        m_VAO.Unbind();

	}
	~Mesh();

    void SetupMesh() {



    }

    void Draw(const Renderer::Shader& shader) {




    }








};

