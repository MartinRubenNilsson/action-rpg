#include "physics_debug.h"

namespace physics
{
	sf::Color _b2_to_sf(const b2Color& color)
	{
		return sf::Color(
			static_cast<sf::Uint8>(color.r * 255),
			static_cast<sf::Uint8>(color.g * 255),
			static_cast<sf::Uint8>(color.b * 255),
			static_cast<sf::Uint8>(color.a * 255));
	};

	DebugDrawSFML::DebugDrawSFML(sf::RenderWindow& window)
		: _window(window)
	{
	}

	void DebugDrawSFML::SetTileSize(const sf::Vector2u& tile_size) {
		_tile_size = tile_size;
	}

	void DebugDrawSFML::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
	{
		sf::ConvexShape polygon(vertexCount);
		for (int32 i = 0; i < vertexCount; ++i)
			polygon.setPoint(i, _world_to_pixels(vertices[i]));
		polygon.setFillColor(sf::Color::Transparent);
		polygon.setOutlineThickness(1.0f);
		polygon.setOutlineColor(_b2_to_sf(color));
		_window.draw(polygon);
	}

	void DebugDrawSFML::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
	{
		sf::ConvexShape polygon(vertexCount);
		for (int32 i = 0; i < vertexCount; ++i)
			polygon.setPoint(i, _world_to_pixels(vertices[i]));
		polygon.setFillColor(_b2_to_sf(color));
		_window.draw(polygon);
	}

	void DebugDrawSFML::DrawCircle(const b2Vec2& center, float radius, const b2Color& color)
	{
		float radius_in_pixels = radius * _tile_size.x;

		sf::CircleShape circle(radius_in_pixels);
		circle.setPosition(_world_to_pixels(center));
		circle.setOrigin(radius_in_pixels, radius_in_pixels);
		circle.setFillColor(sf::Color::Transparent);
		circle.setOutlineThickness(1.0f);
		circle.setOutlineColor(_b2_to_sf(color));
		_window.draw(circle);
	}

	void DebugDrawSFML::DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color)
	{
		// TODO: What is axis?

		float radius_in_pixels = radius * _tile_size.x;

		sf::CircleShape circle(radius_in_pixels);
		circle.setPosition(_world_to_pixels(center));
		circle.setOrigin(radius_in_pixels, radius_in_pixels);
		circle.setFillColor(_b2_to_sf(color));
		_window.draw(circle);
	}

	void DebugDrawSFML::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
	{
	}

	void DebugDrawSFML::DrawTransform(const b2Transform& xf)
	{
	}

	void DebugDrawSFML::DrawPoint(const b2Vec2& p, float size, const b2Color& color)
	{
	}

	sf::Vector2f DebugDrawSFML::_world_to_pixels(const b2Vec2& vec) const
	{
		return sf::Vector2f(
			vec.x * _tile_size.x,
			vec.y * _tile_size.y);
	}
}

