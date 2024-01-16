#pragma once

#include <iostream>
#include <filesystem>
#include <string>

// OpenGL Extension Wrangler
#include <GL/glew.h> 
#include <GL/wglew.h> //WGLEW = Windows GL Extension Wrangler (change for different platform) 

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/string_cast.hpp>


class ShaderProgram {
public:
	// you can add more constructors for pipeline with GS, TS etc.
	ShaderProgram(void) = default;
	ShaderProgram(const std::filesystem::path& VS_file, const std::filesystem::path& FS_file);

	void activate(void) { glUseProgram(ID); };
	void deactivate(void) { glUseProgram(0); };
	void clear(void) { deactivate();  glDeleteProgram(ID); ID = 0; };

	//void setUniform(const std::string& name, float val);
	//void setUniform(const std::string& name, int val);
	//void setUniform(const std::string& name, glm::vec3 val);

	void setUniform(const std::string& name, const glm::vec4& in_vec4) {
		auto loc = getUniformLocation(name);
		if (loc >= 0) {
			glUniform4fv(loc, 1, glm::value_ptr(in_vec4));
		}
	}

	void setUniform(const std::string& name, const glm::mat4& mat4) {
		auto loc = getUniformLocation(name);
		if (loc >= 0) {
			glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mat4));
		}
	}

private:
	GLuint ID;

	GLint getUniformLocation(const std::string& name) {
		auto loc = glGetUniformLocation(ID, name.c_str());
		if (loc == -1) {
			std::cout << "no:" << name << '\n';
		}
		return loc;
	}

	std::string textFileRead(const std::filesystem::path& fn);
	std::string getShaderInfoLog(const GLuint obj);
	std::string getProgramInfoLog(const GLuint obj);

	GLuint compile_shader(const std::filesystem::path& source_file, const GLenum type);
	GLuint link_shader(const std::vector<GLuint> shader_ids);
};
