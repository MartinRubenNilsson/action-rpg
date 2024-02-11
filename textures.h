#pragma once

namespace textures
{
	extern const std::filesystem::path ERROR_TEXTURE_PATH;
	extern bool log_errors;

	// The texture is loaded from file if it's not already in the cache.
	// If the texture fails to load, an error texture is returned instead.
	std::shared_ptr<sf::Texture> get(const std::filesystem::path& path);
	void clear_cache();
}
