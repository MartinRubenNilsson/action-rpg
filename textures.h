#pragma once

namespace textures
{
	extern bool log_errors;

	void shutdown();

	std::shared_ptr<sf::Texture> get_error_texture();
	std::shared_ptr<sf::Texture> create_checkerboard_texture(
		const sf::Vector2u& size, const sf::Vector2u& tile_size, const sf::Color& color1, const sf::Color& color2);

	// TEXTURE CACHE

	// Loads a texture from file, or returns a cached version if it has already been loaded.
	std::shared_ptr<sf::Texture> load_cached_texture(const std::filesystem::path& path);
	void clear_texture_cache();

	// RENDER TEXTURE POOL

	// Searches the pool for a render texture of the specified size and returns it,
	// or creates a new one if none are available. When done with the texture, call
	// give_render_texture_to_pool() to return it to the pool.
	std::unique_ptr<sf::RenderTexture> take_render_texture_from_pool(const sf::Vector2u& size);
	void give_render_texture_to_pool(std::unique_ptr<sf::RenderTexture> texture);
	void clear_render_texture_pool();
}
