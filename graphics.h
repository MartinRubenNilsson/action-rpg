#pragma once

namespace graphics
{
	void initialize();

	// SHADERS

	//Returns -1 on failure.
	int load_shader(const std::filesystem::path& vertex_shader_path, const std::filesystem::path& fragment_shader_path);
	//Pass -1 to unbind the current shader.
	void bind_shader(int shader_id = -1);
	//You need to bind the shader before calling these functions!
	void set_shader_uniform_1f(int shader_id, const std::string& name, float x);
	void set_shader_uniform_2f(int shader_id, const std::string& name, float x, float y);
	void set_shader_uniform_3f(int shader_id, const std::string& name, float x, float y, float z);
	void set_shader_uniform_4f(int shader_id, const std::string& name, float x, float y, float z, float w);
	void set_shader_uniform_1i(int shader_id, const std::string& name, int x);
	void set_shader_uniform_2i(int shader_id, const std::string& name, int x, int y);

	// TEXTURES (todo)
}