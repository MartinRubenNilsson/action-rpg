#include "stdafx.h"
#include "graphics.h"
#include "console.h"
#include <glad/glad.h>
#pragma comment(lib, "opengl32")

namespace graphics
{
	struct Shader
	{
		std::filesystem::path vertex_shader_path;
		std::filesystem::path fragment_shader_path;
		std::string vertex_shader_bytecode;
		std::string fragment_shader_bytecode;
		int vertex_shader_id = -1;
		int fragment_shader_id = -1;
		//TODO
	};

	std::vector<Shader> _shaders;
	std::unordered_map<std::string, int> _shader_handle_to_id;

	int shader_load(const std::filesystem::path& vertex_shader_path, const std::filesystem::path& fragment_shader_path)
	{
		// CHECK CACHE FOR EXISTING SHADER
		
		const std::string shader_handle = vertex_shader_path.string() + ":" + fragment_shader_path.string();
		const auto it = _shader_handle_to_id.find(shader_handle);
		if (it != _shader_handle_to_id.end()) {
			return it->second;
		}

		// LOAD AND CREATE VERTEX SHADER

		std::ifstream vertex_shader_file{ vertex_shader_path };
		if (!vertex_shader_file) {
			console::log_error("Failed to open vertex shader file: " + vertex_shader_path.string());
			return -1;
		}
		std::string vertex_shader_bytecode{ std::istreambuf_iterator<char>(vertex_shader_file), std::istreambuf_iterator<char>() };
		const int vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);

		return -1;
	}

}