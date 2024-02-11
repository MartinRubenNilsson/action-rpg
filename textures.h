#pragma once

namespace textures
{
	extern const std::filesystem::path ERROR_TEXTURE_PATH;
	extern bool log_errors;

	std::shared_ptr<sf::Texture> get(const std::filesystem::path& path);
	void clear_cache();
}
