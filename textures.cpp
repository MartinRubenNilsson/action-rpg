#include "stdafx.h"
#include "textures.h"
#include "console.h"
#include "graphics.h"

namespace textures
{
	extern bool log_errors =
#ifdef _DEBUG
		true;
#else
		false;
#endif

	struct TextureAsset
	{
		enum State
		{
			NotLoaded,
			Loaded,
			Error
		};

		std::shared_ptr<sf::Texture> texture;
		State state = NotLoaded;

		void load(const std::string& filename)
		{
			texture = std::make_shared<sf::Texture>();
			state = texture->loadFromFile(filename) ? Loaded : Error;
			if (state == Error && log_errors)
				console::log_error("Failed to load texture: " + filename);
		}
	};

	std::shared_ptr<sf::Texture> _error_texture;
	std::unordered_map<std::filesystem::path, TextureAsset> _texture_cache;
	std::vector<std::unique_ptr<sf::RenderTexture>> _render_texture_pool;

	void shutdown()
	{
		_error_texture.reset();
		clear_texture_cache();
		clear_render_texture_pool();
	}

	std::shared_ptr<sf::Texture> get_error_texture()
	{
		if (!_error_texture)
			_error_texture = create_checkerboard_texture(
				{ 512, 512 }, { 64, 64 }, sf::Color::Black, sf::Color::Magenta);
		return _error_texture;
	}

	std::shared_ptr<sf::Texture> create_checkerboard_texture(
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
		graphics::bind_shader();
		return std::make_shared<sf::Texture>(render_texture.getTexture());
	}

	std::shared_ptr<sf::Texture> load_cached_texture(const std::filesystem::path& path)
	{
		std::filesystem::path normal_path = path.lexically_normal();
		TextureAsset& asset = _texture_cache[normal_path];
		if (asset.state == TextureAsset::NotLoaded) {
			asset.load(normal_path.string());
			if (asset.state == TextureAsset::Error)
				asset.texture = get_error_texture();
		}
		return asset.texture;
	}

	void clear_texture_cache() {
		_texture_cache.clear();
	}

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