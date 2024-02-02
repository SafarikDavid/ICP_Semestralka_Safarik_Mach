#pragma once
#ifndef OBJloader_H
#define OBJloader_H

#include <vector>
#include <filesystem>
#include <glm/fwd.hpp>

bool loadOBJ(
	const std::filesystem::path& path,
	std::vector < glm::vec3 > & out_vertices,
	std::vector < glm::vec2 > & out_uvs,
	std::vector < glm::vec3 > & out_normals
);

bool loadOBJ(const std::filesystem::path& path, std::vector < glm::vec3 >& out_vertices, std::vector < glm::vec2 >& out_uvs, std::vector < glm::vec3 >& out_normals, std::vector<GLuint>& out_indices);

#endif
