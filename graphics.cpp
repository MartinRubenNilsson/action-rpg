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
		uint32_t vertex_shader_object = 0;
		uint32_t fragment_shader_object = 0;
		uint32_t program_object = 0;
	};

	std::vector<Shader> _shaders;
	std::unordered_map<std::string, int> _shader_handle_to_id;

	Shader* shader_get(int shader_id)
	{
		if (shader_id < 0 || shader_id >= (int)_shaders.size())
			return nullptr;
		return &_shaders[shader_id];
	}

	int shader_load(const std::filesystem::path& vertex_shader_path, const std::filesystem::path& fragment_shader_path)
	{
		// CHECK CACHE FOR EXISTING SHADER
		
		const std::string shader_handle = vertex_shader_path.string() + ":" + fragment_shader_path.string();
		const auto it = _shader_handle_to_id.find(shader_handle);
		if (it != _shader_handle_to_id.end()) {
			return it->second;
		}

		int success;

		// LOAD AND CREATE VERTEX SHADER

		std::ifstream vertex_shader_file{ vertex_shader_path };
		if (!vertex_shader_file) {
			console::log_error("Failed to open vertex shader file: " + vertex_shader_path.string());
			return -1;
		}
		std::string vertex_shader_bytecode{ std::istreambuf_iterator<char>(vertex_shader_file), std::istreambuf_iterator<char>() };
		const int vertex_shader_object = glCreateShader(GL_VERTEX_SHADER);
		const char* vertex_shader_bytecode_cstr = vertex_shader_bytecode.c_str();
		glShaderSource(vertex_shader_object, 1, &vertex_shader_bytecode_cstr, nullptr);
		glCompileShader(vertex_shader_object);
		glGetShaderiv(vertex_shader_object, GL_COMPILE_STATUS, &success);
		if (!success) {
			char info_log[512];
			glGetShaderInfoLog(vertex_shader_object, sizeof(info_log), nullptr, info_log);
			console::log_error("Failed to compile vertex shader: " + vertex_shader_path.string() + "\n" + info_log);
			return -1;
		}

		// LOAD AND CREATE FRAGMENT SHADER

		std::ifstream fragment_shader_file{ fragment_shader_path };
		if (!fragment_shader_file) {
			console::log_error("Failed to open fragment shader file: " + fragment_shader_path.string());
			return -1;
		}
		std::string fragment_shader_bytecode{ std::istreambuf_iterator<char>(fragment_shader_file), std::istreambuf_iterator<char>() };
		const int fragment_shader_object = glCreateShader(GL_FRAGMENT_SHADER);
		const char* fragment_shader_bytecode_cstr = fragment_shader_bytecode.c_str();
		glShaderSource(fragment_shader_object, 1, &fragment_shader_bytecode_cstr, nullptr);
		glCompileShader(fragment_shader_object);
		glGetShaderiv(fragment_shader_object, GL_COMPILE_STATUS, &success);
		if (!success) {
			char info_log[512];
			glGetShaderInfoLog(fragment_shader_object, sizeof(info_log), nullptr, info_log);
			console::log_error("Failed to compile fragment shader: " + fragment_shader_path.string() + "\n" + info_log);
			return -1;
		}

		// CREATE PROGRAM OBJECT

		const int program_object = glCreateProgram();
		glAttachShader(program_object, vertex_shader_object);
		glAttachShader(program_object, fragment_shader_object);
		glLinkProgram(program_object);
		glGetProgramiv(program_object, GL_LINK_STATUS, &success);
		if (!success) {
			char info_log[512];
			glGetProgramInfoLog(program_object, sizeof(info_log), nullptr, info_log);
			console::log_error("Failed to link program object: " + vertex_shader_path.string() + ", " +
				fragment_shader_path.string() + "\n" + info_log);
			return -1;
		}

		// STORE SHADER

		const int shader_id = (int)_shaders.size();
		Shader& shader = _shaders.emplace_back();
		shader.vertex_shader_path = vertex_shader_path;
		shader.fragment_shader_path = fragment_shader_path;
		shader.vertex_shader_bytecode = std::move(vertex_shader_bytecode);
		shader.fragment_shader_bytecode = std::move(fragment_shader_bytecode);
		shader.vertex_shader_object = vertex_shader_object;
		shader.fragment_shader_object = fragment_shader_object;
		shader.program_object = program_object;
		_shader_handle_to_id[shader_handle] = shader_id;

		return shader_id;
	}

	void shader_use(int shader_id)
	{
		if (const Shader* shader = shader_get(shader_id)) {
			glUseProgram(shader->program_object);
		}
	}
}