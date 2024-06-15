#pragma once

namespace graphics
{
	void initialize();

	//Returns -1 on failure.
	int shader_load(const std::filesystem::path& vertex_shader_path, const std::filesystem::path& fragment_shader_path);
	void shader_use(int shader_id);
}