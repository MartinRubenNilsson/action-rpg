#pragma once

namespace background
{
	enum class Type
	{
		None,
		MountainDusk,
	};

	void set_type(Type type);
	void update(float dt);
	void add_sprites_to_render_queue(const sf::Vector2f& view_min, const sf::Vector2f& view_max);
}

