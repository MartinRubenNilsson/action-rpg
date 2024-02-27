#include "stdafx.h"
#include "textures.h"
#include "console.h"

namespace textures
{
	const std::filesystem::path ERROR_TEXTURE_PATH = "assets/textures/error.png";

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

	std::unordered_map<std::filesystem::path, TextureAsset> _texture_cache;
	std::vector<std::unique_ptr<sf::RenderTexture>> _render_texture_pool;

	std::shared_ptr<sf::Texture> load_texture(const std::filesystem::path& path)
	{
		std::filesystem::path normal_path = path.lexically_normal();
		TextureAsset& asset = _texture_cache[normal_path];
		if (asset.state == TextureAsset::NotLoaded) {
			asset.load(normal_path.string());
			if (asset.state == TextureAsset::Error && normal_path != ERROR_TEXTURE_PATH)
				asset.texture = load_texture(ERROR_TEXTURE_PATH);
		}
		return asset.texture;
	}

	void clear_texture_cache() {
		_texture_cache.clear();
	}

	std::unique_ptr<sf::RenderTexture> get_render_texture(const sf::Vector2u& size)
	{
		for (auto it = _render_texture_pool.begin(); it != _render_texture_pool.end(); ++it) {
			if ((*it)->getSize() != size) continue;
			std::unique_ptr<sf::RenderTexture> texture = std::move(*it);
			_render_texture_pool.erase(it);
			return texture;
		}
		auto texture = std::make_unique<sf::RenderTexture>();
		bool created = texture->create(size.x, size.y);
		assert(created);
		return texture;
	}

	void recycle_render_texture(std::unique_ptr<sf::RenderTexture> texture) {
		_render_texture_pool.push_back(std::move(texture));
	}

	void clear_render_texture_pool() {
		_render_texture_pool.clear();
	}
}