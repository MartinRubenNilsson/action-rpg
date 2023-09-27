#pragma once

class b2Body;

void         set_position(b2Body& body, const sf::Vector2f& position);
sf::Vector2f get_position(const b2Body& body); // Typically the top-left corner.
void         set_world_center(b2Body& body, const sf::Vector2f& center); // Center of mass.
sf::Vector2f get_world_center(const b2Body& body); // Center of mass.
void         set_linear_velocity(b2Body& body, const sf::Vector2f& velocity);
sf::Vector2f get_linear_velocity(const b2Body& body);
float        get_linear_speed(const b2Body& body);

