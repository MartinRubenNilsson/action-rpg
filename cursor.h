#pragma once

namespace cursor
{
	void render(sf::RenderTarget& target);

	void set_visible(bool visible);
	void set_position(const sf::Vector2f& position);
	void set_scale(float scale);
}
