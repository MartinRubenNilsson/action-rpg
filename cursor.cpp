#include "stdafx.h"
#include "cursor.h"
#include "graphics.h"
#include "sprites.h"

namespace cursor
{
	bool _visible = true;
	Vector2f _tex_min{ 0.f, 0.f }; // in pixels
	Vector2f _tex_max{ 16.f, 16.f }; // in pixels
	Vector2f _position;
	float _scale = 1.f;

	void render_sprite()
	{
		if (!_visible) return;

		const graphics::TextureHandle texture = graphics::load_texture("assets/textures/cursors/cursor.png");
		if (texture == graphics::TextureHandle::Invalid) return;
		Vector2u texture_size;
		graphics::get_texture_size(texture, texture_size.x, texture_size.y);

		sprites::Sprite sprite{};
		sprite.texture = texture;
		sprite.min = _position;
		sprite.max = _position + _scale * Vector2f{ 16.f, 16.f };
		sprite.tex_min = _tex_min;
		sprite.tex_max = _tex_max;
		sprite.tex_min /= texture_size;
		sprite.tex_max /= texture_size;
		sprites::add_sprite_to_render_queue(sprite);
		sprites::render("Cursor");
	}

	void set_visible(bool visible) {
		_visible = visible;
	}

	void set_position(const Vector2f& position) {
		_position = position;
	}

	void set_scale(float scale) {
		_scale = scale;
	}
}