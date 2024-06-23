#include "stdafx.h"
#include "background.h"
#include "graphics.h"
#include "sprites.h"
#include "console.h"

namespace background
{
	const std::string _MOUNTAIN_DUSK_TEXTURE_PATHS[] = {
		"assets/textures/backgrounds/mountain_dusk/sky.png",
		"assets/textures/backgrounds/mountain_dusk/far-clouds.png",
		"assets/textures/backgrounds/mountain_dusk/near-clouds.png",
		"assets/textures/backgrounds/mountain_dusk/far-mountains.png",
		"assets/textures/backgrounds/mountain_dusk/mountains.png",
		"assets/textures/backgrounds/mountain_dusk/trees.png",
	};

	struct Layer
	{
		int texture_id = -1;
		unsigned int texture_width = 0;
		unsigned int texture_height = 0;
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
			for (const std::string& path : _MOUNTAIN_DUSK_TEXTURE_PATHS) {
				const int texture_id = graphics::load_texture(path);
				if (texture_id == -1) continue;
				Layer& layer = _layers.emplace_back();
				layer.texture_id = texture_id;
				graphics::get_texture_size(texture_id, layer.texture_width, layer.texture_height);
			}
			break;
		}
	}

	void update(float dt)
	{
		for (size_t i = 0; i < _layers.size(); ++i) {
			Layer& layer = _layers[i];
			layer.offset += i * i * i * dt;
			if (layer.offset >= layer.texture_width)
				layer.offset -= layer.texture_width;
		}
	}

	void render_sprites(const sf::Vector2f& view_min, const sf::Vector2f& view_max)
	{
		sprites::Sprite sprite{};
		for (const Layer& layer : _layers) {
			sprite.texture_id = layer.texture_id;
			for (float x = -layer.offset; x < view_max.x; x += layer.texture_width) {
				sprite.min = { view_min.x + x, view_min.y };
				sprite.max = { view_min.x + x + layer.texture_width, view_min.y + layer.texture_height };
				sprite.tex_min = { 0.f, 0.f };
				sprite.tex_max = { 1.f, 1.f };
				sprites::add_sprite_to_render_queue(sprite);
			}
		}
		sprites::render_sprites_in_render_queue();
	}
}