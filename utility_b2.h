#pragma once

class b2Body;

namespace b2
{
	void         set_position(b2Body& body, const sf::Vector2f& position);
	sf::Vector2f get_position(const b2Body& body);
	void         set_linear_velocity(b2Body& body, const sf::Vector2f& velocity);
	sf::Vector2f get_linear_velocity(const b2Body& body);
	float        get_linear_speed(const b2Body& body);
}

