#pragma once

#include <vector>
#include <iostream>
#include <algorithm>

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
	float alpha = 1.0f;
	GLuint texture = 0;

	glm::vec3 viewPos;
	glm::vec3 viewFront;

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
		// P,V,M
		mesh_shader.setUniform("uPm", projection_matrix);
		mesh_shader.setUniform("uVm", view_matrix);
		mesh_shader.setUniform("uMm", model_matrix);
		// mesh_shader.setUniform("diffuse_material", diffuse_material);
		// mesh_shader.setUniform("ambient_material", ambient_material);
		// Material
		mesh_shader.setUniform("specular_material", specular_material);
		mesh_shader.setUniform("shininess", shininess);
		mesh_shader.setUniform("alpha", alpha);

		mesh_shader.setUniform("viewPos", viewPos);

		mesh_shader.setUniform("pointLight.position", glm::vec3(view_matrix * glm::vec4(glm::vec3(5.0f, 10.0f, 5.0f), 1.0))); // Transform world-space light position to view-space light position
		// mesh_shader.setUniform("pointLight.position", glm::vec3(5.0f, 10.0f, 5.0f));
		mesh_shader.setUniform("pointLight.ambient", glm::vec3(.5f));
		mesh_shader.setUniform("pointLight.diffuse", glm::vec3(.5f));
		mesh_shader.setUniform("pointLight.specular", glm::vec3(.5f));
		mesh_shader.setUniform("pointLight.constant", 1.0f);
		mesh_shader.setUniform("pointLight.linear", 0.045f);
		mesh_shader.setUniform("pointLight.quadratic", 0.0075f);

		// Ambient light
		mesh_shader.setUniform("ambientLight.ambient", glm::vec3(0.05f));
		mesh_shader.setUniform("ambientLight.diffuse", glm::vec3(0.05f));
		mesh_shader.setUniform("ambientLight.specular", glm::vec3(0.05f));

		// Spotlight - Fleshlight
		mesh_shader.setUniform("spotLight.position", glm::vec3(view_matrix * glm::vec4(viewPos, 1.0)));
		mesh_shader.setUniform("spotLight.direction", glm::vec3(view_matrix * glm::vec4(viewFront, 0.0)));

		mesh_shader.setUniform("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
		mesh_shader.setUniform("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));

		mesh_shader.setUniform("spotLight.ambient", glm::vec3(1.0f));
		mesh_shader.setUniform("spotLight.diffuse", glm::vec3(1.0f));
		mesh_shader.setUniform("spotLight.specular", glm::vec3(1.0f));

		mesh_shader.setUniform("spotLight.constant", 1.0f);
		mesh_shader.setUniform("spotLight.linear", 0.22f);
		mesh_shader.setUniform("spotLight.quadratic", 0.20f);

		// Directional light - Sun
		mesh_shader.setUniform("directionalLight.direction", glm::vec3(view_matrix * glm::vec4(glm::vec3(-0.2f, -1.0f, -0.3f), 0.0)));
		mesh_shader.setUniform("directionalLight.ambient", glm::vec3(0.1f));
		mesh_shader.setUniform("directionalLight.diffuse", glm::vec3(0.2f));
		mesh_shader.setUniform("directionalLight.specular", glm::vec3(0.3f));


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

	glm::vec3 calculateDimensions(float scale = 1.0f){
		glm::vec3 firstPos = vertices[0].position;
		float minX = firstPos.x, minY = firstPos.y, minZ = firstPos.z, maxX = firstPos.x, maxY = firstPos.y, maxZ = firstPos.z;
		for (const vertex vert : vertices) {
			minX = (vert.position.x < minX) ? vert.position.x : minX;
			minY = (vert.position.y < minY) ? vert.position.y : minY;
			minZ = (vert.position.z < minZ) ? vert.position.z : minZ;

			maxX = (vert.position.x > maxX) ? vert.position.x : maxX;
			maxY = (vert.position.y > maxY) ? vert.position.y : maxY;
			maxZ = (vert.position.z > maxZ) ? vert.position.z : maxZ;
		}
		return glm::vec3(maxX - minX, maxY - minY, maxZ - minZ)*scale;
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

