#pragma once

namespace graphics
{
	//Returns -1 on failure.
	int shader_load(const std::filesystem::path& vertex_shader_path, const std::filesystem::path& fragment_shader_path);
}