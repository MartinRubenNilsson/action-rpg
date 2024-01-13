#include "stdafx.h"
#include "background.h"
#include "console.h"

namespace background
{
	Type type = Type::None;

	enum class MountainDuskLayerType
	{
		Sky,
		FarClouds,
		NearClouds,
		FarMountains,
		Mountains,
		Trees,
		Count,
	};

	struct MountainDuskLayer
	{
		sf::Texture texture;
		float offset = 0.f;
	};

	std::array<MountainDuskLayer, (size_t)MountainDuskLayerType::Count> _mountain_dusk_layers;

	void _load_mountain_dusk_assets()
	{
		std::array<std::filesystem::path, (size_t)MountainDuskLayerType::Count> paths;
		paths[(size_t)MountainDuskLayerType::Sky] = "assets/backgrounds/mountain_dusk/sky.png";
		paths[(size_t)MountainDuskLayerType::FarClouds] = "assets/backgrounds/mountain_dusk/far-clouds.png";
		paths[(size_t)MountainDuskLayerType::NearClouds] = "assets/backgrounds/mountain_dusk/near-clouds.png";
		paths[(size_t)MountainDuskLayerType::FarMountains] = "assets/backgrounds/mountain_dusk/far-mountains.png";
		paths[(size_t)MountainDuskLayerType::Mountains] = "assets/backgrounds/mountain_dusk/mountains.png";
		paths[(size_t)MountainDuskLayerType::Trees] = "assets/backgrounds/mountain_dusk/trees.png";
		for (size_t i = 0; i < paths.size(); ++i) {
			MountainDuskLayer& layer = _mountain_dusk_layers[i];
			if (!layer.texture.loadFromFile(paths[i].string()))
				console::log("Failed to load background asset: " + paths[i].string());
		}
	}

	void load_assets()
	{
		_load_mountain_dusk_assets();
	}

	void unload_assets()
	{
		_mountain_dusk_layers = {};
	}

	void update(float dt)
	{
		switch (type) {
		case Type::MountainDusk:
			for (size_t i = 0; i < (size_t)MountainDuskLayerType::Count; ++i) {
				MountainDuskLayer& layer = _mountain_dusk_layers[i];
				const unsigned int texture_width = layer.texture.getSize().x;
				layer.offset += i * i * i * dt;
				if (layer.offset >= texture_width)
					layer.offset -= texture_width;
			}
		}
	}

	void render(sf::RenderTarget& target)
	{
		const unsigned int target_width = target.getSize().x;
		switch (type) {
		case Type::MountainDusk:
			for (size_t i = 0; i < (size_t)MountainDuskLayerType::Count; ++i) {
				const MountainDuskLayer& layer = _mountain_dusk_layers[i];
				const unsigned int texture_width = layer.texture.getSize().x;
				sf::Sprite sprite(layer.texture);
				for (float x = -layer.offset; x < target_width; x += texture_width) {
					sprite.setPosition(x, 0.f);
					target.draw(sprite);
				}
			}
		}
	}
}