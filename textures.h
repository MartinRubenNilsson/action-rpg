#pragma once

namespace textures
{
	extern const std::filesystem::path ERROR_TEXTURE_PATH;

	extern bool log_errors;

	void unload_assets();
	std::shared_ptr<sf::Texture> get_error_texture();
	std::shared_ptr<sf::Texture> get(const std::filesystem::path& path);
}
