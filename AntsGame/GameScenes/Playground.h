#pragma once

#include "src/Sandbox/Scene.h"


#include "src/world/Player.h"
#include "src/world/Sky.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

class Playground : public Scene {


private:

	World::Sky m_sky{World::Sky::SkyboxesType::SAND};
	Player m_player; // TODO put player in namespace for consistency


	float m_realTime = 0;


	unsigned int m_shader;

	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;


public:

	Playground() {

		float vertices[] = {
			 -.5f, -.5f, -.5f ,
			 + .5f, -.5f, -.5f,
			 + .5f, +.5f, -.5f,
			 - .5f, +.5f, -.5f,
			 - .5f, -.5f, +.5f,
			 + .5f, -.5f, +.5f,
			 + .5f, +.5f, +.5f,
			 - .5f, +.5f, +.5f
		};
		

		unsigned int indices[] = {
			  0, 3, 1, 1, 3, 2,
			  1, 2, 5, 5, 2, 6,
			  5, 6, 4, 4, 6, 7,
			  4, 7, 0, 0, 7, 3,
			  3, 7, 2, 2, 7, 6,
			  4, 0, 5, 5, 0, 1
		};

		glGenBuffers(1, &VBO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glGenBuffers(1, &EBO);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
		const char* vertexShaderSource = "#version 330 core\n"
			"layout (location = 0) in vec3 aPos;\n"
			"out vec3 o_pos;\n"
			"uniform mat4 u_M;\n"
			"uniform mat4 u_VP;\n"
			"void main()\n"
			"{\n"
			"   o_pos = aPos;\n"
			"   gl_Position = u_VP * u_M * vec4(aPos, 1.0);\n"
			"}\0";

		unsigned int vertexShader;
		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
		glCompileShader(vertexShader);

		const char* fragmentShaderSource = "#version 330 core\n"
			"out vec4 FragColor;\n"
			"in vec3 o_pos;\n"
			"void main()\n"
			"{\n"
			"   FragColor = vec4(o_pos.x, o_pos.y, o_pos.z, 1.0f);\n"
			"}\0";

		unsigned int fragmentShader;
		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
		glCompileShader(fragmentShader);


		m_shader = glCreateProgram();
		glAttachShader(m_shader, vertexShader);
		glAttachShader(m_shader, fragmentShader);
		glLinkProgram(m_shader);

		glGenVertexArrays(1, &VAO);

		// 1. bind Vertex Array Object
		glBindVertexArray(VAO);
		// 2. copy our vertices array in a vertex buffer for OpenGL to use
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		// 3. copy our index array in a element buffer for OpenGL to use
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		// 4. then set the vertex attributes pointers
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);



	}


	void step(float delta) override {

		m_player.step(delta);
		m_realTime += delta;

	}

	void onRender() override {

		Renderer::clear();

		glDisable(GL_CULL_FACE);
		glUseProgram(m_shader);

		glUniformMatrix4fv(glGetUniformLocation(m_shader,"u_M"), 1, GL_FALSE, glm::value_ptr(glm::scale(glm::mat4(1), {3,3,3})));
		glUniformMatrix4fv(glGetUniformLocation(m_shader,"u_VP"), 1, GL_FALSE, glm::value_ptr(getCamera().getViewProjectionMatrix()));

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		//m_sky.render(getCamera());

		Renderer::renderDebugCube(getCamera(), { 5,5,5 }, { 3,3,3 }); //??????????????????????????????????? apparitions soudaines parfois


	}


	void onImGuiRender() override {

		ImGui::Text("This is the playground for tests ! ");

	}

	CAMERA_IS_PLAYER(m_player);
};


