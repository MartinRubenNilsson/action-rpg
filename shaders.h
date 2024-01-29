#pragma once

namespace shaders
{
	extern bool log_errors;

	void load_assets(const std::filesystem::path& dir);
	void reload_assets();
	void unload_assets();

	std::shared_ptr<sf::Shader> get(const std::string& name);
}

