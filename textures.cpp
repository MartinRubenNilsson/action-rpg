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
	std::unordered_map<std::filesystem::path, std::shared_ptr<sf::Texture>> textures;

	std::shared_ptr<sf::Texture> get_or_load(const std::filesystem::path& path)
	{
		std::filesystem::path normal_path = path.lexically_normal();
		auto it = textures.find(normal_path);
		if (it != textures.end()) return it->second;
		std::shared_ptr<sf::Texture> texture = std::make_shared<sf::Texture>();
		if (!texture->loadFromFile(normal_path.string())) {
			if (log_errors)
				console::log_error("Failed to load texture: " + normal_path.string());
			return nullptr;
		}
		textures[normal_path] = texture;
		return texture;
	}

	void unload_assets() {
		textures.clear();
	}
}