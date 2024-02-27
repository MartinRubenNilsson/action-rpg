#pragma once

namespace textures
{
	extern const std::filesystem::path ERROR_TEXTURE_PATH;
	extern bool log_errors;

	// TEXTURE CACHE

	std::shared_ptr<sf::Texture> load_texture(const std::filesystem::path& path);
	void clear_texture_cache();

	// RENDER TEXTURE POOL

	std::unique_ptr<sf::RenderTexture> get_render_texture(const sf::Vector2u& size);
	void recycle_render_texture(std::unique_ptr<sf::RenderTexture> texture);
	void clear_render_texture_pool();
}
