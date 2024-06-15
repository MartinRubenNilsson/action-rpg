#pragma once

namespace graphics
{
	void initialize();

	//Returns -1 on failure.
	int shader_load(const std::filesystem::path& vertex_shader_path, const std::filesystem::path& fragment_shader_path);
	void shader_bind(int shader_id);
	void shader_set_uniform_1f(int shader_id, const std::string& name, float x);
	void shader_set_uniform_2f(int shader_id, const std::string& name, float x, float y);
	void shader_set_uniform_3f(int shader_id, const std::string& name, float x, float y, float z);
	void shader_set_uniform_4f(int shader_id, const std::string& name, float x, float y, float z, float w);
	void shader_set_uniform_1i(int shader_id, const std::string& name, int x);
	void shader_set_uniform_2i(int shader_id, const std::string& name, int x, int y);
}