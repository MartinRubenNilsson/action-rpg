#pragma once

namespace shaders
{
	extern bool log_errors;

	void load_shaders(const std::filesystem::path& dir);
	void unload_shaders();

	std::shared_ptr<sf::Shader> get(const std::string& name);
}

