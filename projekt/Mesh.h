#pragma once

#include <vector>
#include <iostream>

// OpenGL Extension Wrangler
#include <GL/glew.h> 
#include <GL/wglew.h> //WGLEW = Windows GL Extension Wrangler (change for different platform) 

// GLFW toolkit
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/string_cast.hpp>

#include "ShaderProgram.h"
#include "OBJloader.h"

//vertex description
struct vertex {
	glm::vec3 position;
	glm::vec2 texcoord;
	glm::vec3 normal;
};

class Mesh
{
public:
	std::vector<vertex> vertices;
	std::vector<GLuint> indices;
	GLuint VAO_ID = 0;
	GLuint VBO_ID = 0;
	GLuint EBO_ID = 0;
	GLenum primitive = GL_POINTS;


	glm::mat4 model_matrix;
	glm::vec4 diffuse_material;
	glm::vec4 ambient_material;
	glm::vec4 specular_material;
	float shininess;
	GLuint texture = 0;
	
	ShaderProgram mesh_shader;

	Mesh(void) = default;

	Mesh(const std::filesystem::path& VS_file, const std::filesystem::path& FS_file, const std::filesystem::path& OBJ_file):mesh_shader(VS_file, FS_file)
	{
		std::vector < glm::vec3 > out_vertices;
		std::vector < glm::vec2 > out_uvs;
		std::vector < glm::vec3 > out_normals;
		
		if (!loadOBJ(OBJ_file, out_vertices, out_uvs, out_normals))
			throw std::exception("OBJload failed");

		for (size_t i = 0; i < out_vertices.size(); ++i) {
			vertices.emplace_back((vertex{ out_vertices.at(i), out_uvs.at(i), out_normals.at(i) }));
		}
		
		primitive = GL_TRIANGLES;

		init_VAO();
	}

	Mesh(const std::filesystem::path& VS_file, const std::filesystem::path& FS_file, std::vector<vertex>& vertices, std::vector<GLuint> & indices):
		Mesh(VS_file,FS_file,vertices)
	{
		this->indices = indices;

		glBindVertexArray(VAO_ID);

		glGenBuffers(1, &EBO_ID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_ID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

		glBindVertexArray(0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	Mesh(const std::filesystem::path& VS_file, const std::filesystem::path& FS_file, std::vector<vertex>& vertices) : vertices(vertices), mesh_shader(VS_file, FS_file)
	{
		init_VAO();
	}

	void draw(const glm::mat4 & projection_matrix, const glm::mat4 & view_matrix, const glm::mat4 & model_matrix) {
		mesh_shader.activate();
		mesh_shader.setUniform("uPm", projection_matrix);
		mesh_shader.setUniform("uVm", view_matrix);
		mesh_shader.setUniform("uMm", model_matrix);
		mesh_shader.setUniform("diffuse_material", diffuse_material);
		mesh_shader.setUniform("ambient_material", ambient_material);
		mesh_shader.setUniform("specular_material", specular_material);
		mesh_shader.setUniform("shininess", shininess);

		mesh_shader.setUniform("uLightPos", glm::vec3(0.0f, 10.0f, 0.0f));
		mesh_shader.setUniform("uLightColor", glm::vec3(1.0f, 1.0f, 1.0f));

		if (texture != 0)
			glBindTexture(GL_TEXTURE_2D, texture);
		glActiveTexture(GL_TEXTURE0);
		mesh_shader.setUniform("ourTexture", 0);

		glBindVertexArray(VAO_ID);
		if (indices.empty())
			glDrawArrays(primitive, 0, vertices.size());
		else
			glDrawElements(primitive, indices.size(), GL_UNSIGNED_INT, 0);
	}

	void draw(const glm::mat4& projection_matrix, const glm::mat4& view_matrix) {
		this->draw(projection_matrix, view_matrix, this->model_matrix);
	}

private:
	void init_VAO(void) {
		// create VAO = data description
		glGenVertexArrays(1, &VAO_ID);
		glBindVertexArray(VAO_ID);

		// create vertex buffer and fill with data
		glGenBuffers(1, &VBO_ID);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_ID);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex), vertices.data(), GL_STATIC_DRAW);

		//explain GPU the memory layout of the data...
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<void*>(0 + offsetof(vertex, position)));
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<void*>(0 + offsetof(vertex, texcoord)));
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<void*>(0 + offsetof(vertex, normal)));
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

};

