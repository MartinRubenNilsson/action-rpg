#include "stdafx.h"
#include "cursor.h"
#include "graphics.h"
#include "sprites.h"

namespace cursor
{
	bool _visible = true;
	sf::IntRect _texture_rect{ 0, 0, 16, 16 };
	sf::Vector2f _position;
	float _scale = 1.f;

	void render_sprite()
	{
		if (!_visible) return;

		const int texture_id = graphics::load_texture("assets/textures/cursors/cursor.png");
		if (texture_id == -1) return;
		sf::Vector2u texture_size;
		graphics::get_texture_size(texture_id, texture_size.x, texture_size.y);

		sprites::Sprite sprite{};
		sprite.texture_id = texture_id;
		sprite.min = _position;
		sprite.max = _position + _scale * sf::Vector2f{ 16.f, 16.f };
		sprite.tex_min = { (float)_texture_rect.left, (float)_texture_rect.top };
		sprite.tex_max = { (float)_texture_rect.left + _texture_rect.width, (float)_texture_rect.top + _texture_rect.height };
		sprite.tex_min /= sf::Vector2f{ texture_size };
		sprite.tex_max /= sf::Vector2f{ texture_size };
		sprites::add_sprite_to_render_queue(sprite);
		sprites::render();
	}

	void set_visible(bool visible) {
		_visible = visible;
	}

	void set_position(const sf::Vector2f& position) {
		_position = position;
	}

	void set_scale(float scale) {
		_scale = scale;
	}
}