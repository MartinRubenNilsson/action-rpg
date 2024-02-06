#include "stdafx.h"
#include "textures.h"
#include "console.h"

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
			Unloaded,
			Loaded,
			Error
		};

		std::shared_ptr<sf::Texture> texture;
		State state = Unloaded;

		void load(const std::string& filename)
		{
			texture = std::make_shared<sf::Texture>();
			state = texture->loadFromFile(filename) ? Loaded : Error;
			if (state == Error && log_errors)
				console::log_error("Failed to load texture: " + filename);
		}
	};

	TextureAsset _error_texture;
	std::unordered_map<std::filesystem::path, TextureAsset> _texture_cache;

	void unload_assets()
	{
		_texture_cache.clear();
		_error_texture = {};
	}

	std::shared_ptr<sf::Texture> get_error_texture()
	{
		if (_error_texture.state == TextureAsset::Unloaded)
			_error_texture.load("assets/textures/error.png");
		return _error_texture.texture;
	}

	std::shared_ptr<sf::Texture> get(const std::filesystem::path& path)
	{
		std::filesystem::path normal_path = path.lexically_normal();
		TextureAsset& asset = _texture_cache[normal_path];
		if (asset.state == TextureAsset::Unloaded) {
			asset.load(normal_path.string());
			if (asset.state == TextureAsset::Error)
				asset.texture = get_error_texture();
		}
		return asset.texture;
	}
}