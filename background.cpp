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

	std::vector<MountainDuskLayer> _mountain_dusk_layers;

	void _load_mountain_dusk_assets()
	{
		const std::string dir = "assets/textures/backgrounds/mountain_dusk/";
		std::array<std::string, (size_t)MountainDuskLayerType::Count> filenames;
		filenames[(size_t)MountainDuskLayerType::Sky] = "sky.png";
		filenames[(size_t)MountainDuskLayerType::FarClouds] = "far-clouds.png";
		filenames[(size_t)MountainDuskLayerType::NearClouds] = "near-clouds.png";
		filenames[(size_t)MountainDuskLayerType::FarMountains] = "far-mountains.png";
		filenames[(size_t)MountainDuskLayerType::Mountains] = "mountains.png";
		filenames[(size_t)MountainDuskLayerType::Trees] = "trees.png";
		_mountain_dusk_layers.resize((size_t)MountainDuskLayerType::Count);
		for (size_t i = 0; i < (size_t)MountainDuskLayerType::Count; ++i) {
			MountainDuskLayer& layer = _mountain_dusk_layers[i];
			std::string path = dir + filenames[i];
			if (!layer.texture.loadFromFile(path))
				console::log_error("Failed to load background asset: " + path);
		}
	}

	void load_assets()
	{
		_load_mountain_dusk_assets();
	}

	void unload_assets()
	{
		_mountain_dusk_layers.clear();
	}

	void update(float dt)
	{
		switch (type) {
		case Type::MountainDusk:
			for (size_t i = 0; i < _mountain_dusk_layers.size(); ++i) {
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
		sf::View view = target.getView();
		sf::FloatRect view_rect = sf::FloatRect(view.getCenter() - view.getSize() / 2.f, view.getSize());

		switch (type) {
		case Type::MountainDusk:
			for (size_t i = 0; i < _mountain_dusk_layers.size(); ++i) {
				const MountainDuskLayer& layer = _mountain_dusk_layers[i];
				const unsigned int texture_width = layer.texture.getSize().x;
				sf::Sprite sprite(layer.texture);
				for (float x = -layer.offset; x < view_rect.width; x += texture_width) {
					sprite.setPosition(view_rect.left + x, view_rect.top);
					target.draw(sprite);
				}
			}
		}
	}
}