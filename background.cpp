#include "stdafx.h"
#include "background.h"
#include "textures.h"
#include "console.h"

namespace background
{
	const std::filesystem::path _MOUNTAIN_DUSK_TEXTURE_PATHS[] = {
		"assets/textures/backgrounds/mountain_dusk/sky.png",
		"assets/textures/backgrounds/mountain_dusk/far-clouds.png",
		"assets/textures/backgrounds/mountain_dusk/near-clouds.png",
		"assets/textures/backgrounds/mountain_dusk/far-mountains.png",
		"assets/textures/backgrounds/mountain_dusk/mountains.png",
		"assets/textures/backgrounds/mountain_dusk/trees.png",
	};

	struct Layer
	{
		std::shared_ptr<sf::Texture> texture;
		float offset = 0.f;
	};

	Type _type = Type::None;
	std::vector<Layer> _layers;

	void set_type(Type type)
	{
		_type = type;
		switch (type) {
		case Type::None:
			_layers.clear();
			break;
		case Type::MountainDusk:
			_layers.clear();
			for (const std::filesystem::path& path : _MOUNTAIN_DUSK_TEXTURE_PATHS) {
				std::shared_ptr<sf::Texture> texture = textures::load_cached_texture(path);
				if (texture) {
					_layers.emplace_back(texture);
				}
			}
			break;
		}
	}

	void update(float dt)
	{
		switch (_type) {
		case Type::MountainDusk:
			for (size_t i = 0; i < _layers.size(); ++i) {
				Layer& layer = _layers[i];
				const unsigned int texture_width = layer.texture->getSize().x;
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

		for (const Layer& layer : _layers) {
			const unsigned int texture_width = layer.texture->getSize().x;
			sf::Sprite sprite(*layer.texture);
			for (float x = -layer.offset; x < view_rect.width; x += texture_width) {
				sprite.setPosition(view_rect.left + x, view_rect.top);
				target.draw(sprite);
			}
		}
	}
}