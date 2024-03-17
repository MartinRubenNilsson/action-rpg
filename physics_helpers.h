#pragma once

class b2Body; // Forward declaration.

entt::entity get_entity(b2Body* body);
void         set_position(b2Body* body, const sf::Vector2f& position); // Typically the top-left corner.
sf::Vector2f get_position(const b2Body* body); // Typically the top-left corner.
void         set_world_center(b2Body* body, const sf::Vector2f& center); // Center of mass.
sf::Vector2f get_world_center(const b2Body* body); // Center of mass.
void         set_linear_velocity(b2Body* body, const sf::Vector2f& velocity);
sf::Vector2f get_linear_velocity(const b2Body* body);
float        get_linear_speed(const b2Body* body);
// Sets the category bits of all fixtures attached to the body.
void         set_category_bits(b2Body* body, uint32_t category_bits);
// Returns the bitwise OR of all category bits of all fixtures attached to the body.
uint32_t     get_category_bits(const b2Body* body);

