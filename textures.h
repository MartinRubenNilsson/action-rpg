#pragma once

namespace textures
{
	extern bool log_errors;

	std::shared_ptr<sf::Texture> get_or_load(const std::filesystem::path& path);
	void unload_assets();
}
