#include "stdafx.h"
#include "cursor.h"
#include "textures.h"

namespace cursor
{
	const std::filesystem::path _CURSOR_TEXTURE_PATH = "assets/textures/cursors/cursor.png";
	bool _visible = true;
	sf::IntRect _texture_rect{ 0, 0, 16, 16 };
	sf::Vector2f _position;
	float _scale = 1.f;

	void render(sf::RenderTarget& target)
	{
		if (!_visible) return;
		std::shared_ptr<sf::Texture> texture = textures::get(_CURSOR_TEXTURE_PATH);
		if (!texture) return;
		sf::Sprite sprite(*texture, _texture_rect);
		sprite.setPosition(_position);
		sprite.setScale(_scale, _scale);
		target.draw(sprite);
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