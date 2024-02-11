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

	std::shared_ptr<sf::Texture> get(const std::filesystem::path& path)
	{
		std::filesystem::path normal_path = path.lexically_normal();
		TextureAsset& asset = _texture_cache[normal_path];
		if (asset.state == TextureAsset::NotLoaded) {
			asset.load(normal_path.string());
			if (asset.state == TextureAsset::Error && normal_path != ERROR_TEXTURE_PATH)
				asset.texture = get(ERROR_TEXTURE_PATH);
		}
		return asset.texture;
	}

	void clear_cache() {
		_texture_cache.clear();
	}
}