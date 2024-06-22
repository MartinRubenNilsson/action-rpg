#include "stdafx.h"
#include "textures.h"
#include "console.h"
#include "graphics.h"

namespace textures
{
	std::vector<std::unique_ptr<sf::RenderTexture>> _render_texture_pool;

	void shutdown()
	{
		clear_render_texture_pool();
	}

	/*std::shared_ptr<sf::Texture> create_checkerboard_texture(
		const sf::Vector2u& size, const sf::Vector2u& tile_size, const sf::Color& color1, const sf::Color& color2)
	{
		const int shader_id = graphics::load_shader({}, "assets/shaders/checkerboard.frag");
		if (shader_id == -1) return nullptr;
		graphics::bind_shader(shader_id);
		graphics::set_shader_uniform_2f(shader_id, "tile_size", (float)tile_size.x, (float)tile_size.y);
		graphics::set_shader_uniform_4f(shader_id, "color1",
			color1.r / 255.f, color1.g / 255.f, color1.b / 255.f, color1.a / 255.f);
		graphics::set_shader_uniform_4f(shader_id, "color2",
			color2.r / 255.f, color2.g / 255.f, color2.b / 255.f, color2.a / 255.f);
		sf::RenderTexture render_texture;
		render_texture.create(size.x, size.y);
		render_texture.clear(sf::Color::Transparent);
		render_texture.draw(sf::Sprite());
		render_texture.display();
		graphics::bind_shader(-1);
		return std::make_shared<sf::Texture>(render_texture.getTexture());
	}*/

	std::unique_ptr<sf::RenderTexture> take_render_texture_from_pool(const sf::Vector2u& size)
	{
		for (auto it = _render_texture_pool.begin(); it != _render_texture_pool.end(); ++it) {
			if ((*it)->getSize() != size) continue;
			std::unique_ptr<sf::RenderTexture> texture = std::move(*it);
			_render_texture_pool.erase(it);
			return texture;
		}
		std::unique_ptr<sf::RenderTexture> texture = std::make_unique<sf::RenderTexture>();
		bool created = texture->create(size.x, size.y);
		assert(created);
		return texture;
	}

	void give_render_texture_to_pool(std::unique_ptr<sf::RenderTexture> texture) {
		_render_texture_pool.push_back(std::move(texture));
	}

	void clear_render_texture_pool() {
		_render_texture_pool.clear();
	}
}