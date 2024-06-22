#pragma once

namespace textures
{
	extern bool log_errors;

	void shutdown();

	// RENDER TEXTURE POOL

	// Searches the pool for a render texture of the specified size and returns it,
	// or creates a new one if none are available. When done with the texture, call
	// give_render_texture_to_pool() to return it to the pool.
	std::unique_ptr<sf::RenderTexture> take_render_texture_from_pool(const sf::Vector2u& size);
	void give_render_texture_to_pool(std::unique_ptr<sf::RenderTexture> texture);
	void clear_render_texture_pool();
}
