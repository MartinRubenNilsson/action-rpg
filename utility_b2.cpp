#include "utility_b2.h"
#include <box2d/b2_math.h>
#include "math_vectors.h"

void set_position(b2Body& body, const sf::Vector2f& position) {
	body.SetTransform(vector_cast<b2Vec2>(position), body.GetAngle());
}

sf::Vector2f get_position(const b2Body& body) {
	return vector_cast<sf::Vector2f>(body.GetPosition());
}

sf::Vector2f get_world_center(const b2Body& body) {
	return vector_cast<sf::Vector2f>(body.GetWorldCenter());
}

void set_linear_velocity(b2Body& body, const sf::Vector2f& velocity) {
	body.SetLinearVelocity(vector_cast<b2Vec2>(velocity));
}

sf::Vector2f get_linear_velocity(const b2Body& body) {
	return vector_cast<sf::Vector2f>(body.GetLinearVelocity());
}

float get_linear_speed(const b2Body& body) {
	return length(get_linear_velocity(body));
}

