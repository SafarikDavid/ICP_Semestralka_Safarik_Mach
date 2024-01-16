#include <iostream>
#include <fstream>
#include <sstream>

#include "ShaderProgram.h"

ShaderProgram::ShaderProgram(const std::filesystem::path& VS_file, const std::filesystem::path& FS_file)
{
	std::vector<GLuint> shader_names;

	shader_names.push_back(compile_shader(VS_file, GL_VERTEX_SHADER));
	shader_names.push_back(compile_shader(FS_file, GL_FRAGMENT_SHADER));

	ID = link_shader(shader_names);
}

GLuint ShaderProgram::compile_shader(const std::filesystem::path& source_file, const GLenum type)
{
	GLuint name;

	name = glCreateShader(type);
	std::string src = textFileRead(source_file);
	const char* src_string = src.c_str();
	glShaderSource(name, 1, &src_string, NULL);

	glCompileShader(name);

	//get status
	{
		GLint success = 0;
		glGetShaderiv(name, GL_COMPILE_STATUS, &success);
		if (!success) {
			//std::cout << getShaderInfoLog(name);
			throw std::exception(getShaderInfoLog(name).c_str());
		}
	}

	return name;
}

GLuint ShaderProgram::link_shader(const std::vector<GLuint> shader_ids)
{
	GLuint prog_h;

	prog_h = glCreateProgram();
	for (auto const& id : shader_ids)
		glAttachShader(prog_h, id);

	glLinkProgram(prog_h);
	//get result status
	{
		GLint success = 0;
		glGetShaderiv(prog_h, GL_LINK_STATUS, &success);
		if (!success)
			std::cout << getProgramInfoLog(prog_h);
	}

	return prog_h;
}


// load text file
std::string ShaderProgram::textFileRead(const std::filesystem::path& fn) {
	std::ifstream file(fn);
	std::stringstream ss;

	if (!file.is_open()) {
		std::cerr << "Error opening file: " << fn << std::endl;
		exit(EXIT_FAILURE);
	}
	else {
		ss << file.rdbuf();
	}

	return std::move(ss.str());
}

// get shader compilation errors
std::string ShaderProgram::getShaderInfoLog(const GLuint obj) {
	int infologLength = 0;
	std::string s;
	glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);
	if (infologLength > 0) {
		std::vector<char> v(infologLength);
		glGetShaderInfoLog(obj, infologLength, NULL, v.data());
		s.assign(begin(v), end(v));
	}
	return s;
}
// get shader linker errors
std::string ShaderProgram::getProgramInfoLog(const GLuint obj) {
	int infologLength = 0;
	std::string s;
	glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength);
	if (infologLength > 0) {
		std::vector<char> v(infologLength);
		glGetProgramInfoLog(obj, infologLength, NULL, v.data());
		s.assign(begin(v), end(v));
	}
	return s;
}

