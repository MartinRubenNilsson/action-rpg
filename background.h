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
	void render_sprites(const sf::Vector2f& camera_min, const sf::Vector2f& camera_max);
}

