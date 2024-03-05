#pragma once

namespace textures
{
	extern bool log_errors;

	void shutdown();

	std::shared_ptr<sf::Texture> get_error_texture();
	std::shared_ptr<sf::Texture> create_checkerboard_texture(
		const sf::Vector2u& size, const sf::Vector2u& tile_size, const sf::Color& color1, const sf::Color& color2);

	// TEXTURE CACHE

	std::shared_ptr<sf::Texture> load_texture(const std::filesystem::path& path);
	void clear_texture_cache();

	// RENDER TEXTURE POOL

	std::unique_ptr<sf::RenderTexture> get_render_texture(const sf::Vector2u& size);
	void recycle_render_texture(std::unique_ptr<sf::RenderTexture> texture);
	void clear_render_texture_pool();
}
