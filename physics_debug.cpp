#include "physics_debug.h"

namespace physics
{
	// TODO: frustum culling

	sf::Color _b2_to_sf(const b2Color& color)
	{
		return sf::Color(
			static_cast<sf::Uint8>(color.r * 255),
			static_cast<sf::Uint8>(color.g * 255),
			static_cast<sf::Uint8>(color.b * 255),
			static_cast<sf::Uint8>(color.a * 255));
	};

	DebugDrawSFML::DebugDrawSFML(sf::RenderTarget& render_target)
		: _render_target(render_target)
	{
	}

	void DebugDrawSFML::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
	{
		sf::ConvexShape polygon(vertexCount);
		for (int32 i = 0; i < vertexCount; ++i)
		{
			float px_x = vertices[i].x * PIXELS_PER_METER;
			float px_y = vertices[i].y * PIXELS_PER_METER;
			polygon.setPoint(i, sf::Vector2f(px_x, px_y));
		}
		polygon.setFillColor(sf::Color::Transparent);
		polygon.setOutlineThickness(1.0f);
		polygon.setOutlineColor(_b2_to_sf(color));
		_render_target.draw(polygon);
	}

	void DebugDrawSFML::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
	{
		sf::ConvexShape polygon(vertexCount);
		for (int32 i = 0; i < vertexCount; ++i)
		{
			float px_x = vertices[i].x * PIXELS_PER_METER;
			float px_y = vertices[i].y * PIXELS_PER_METER;
			polygon.setPoint(i, sf::Vector2f(px_x, px_y));
		}
		polygon.setFillColor(_b2_to_sf(color));
		_render_target.draw(polygon);
	}

	void DebugDrawSFML::DrawCircle(const b2Vec2& center, float radius, const b2Color& color)
	{
		float px_center_x = center.x * PIXELS_PER_METER;
		float px_center_y = center.y * PIXELS_PER_METER;
		float px_radius = radius * PIXELS_PER_METER;

		sf::CircleShape circle(px_radius);
		circle.setPosition(px_center_x, px_center_y);
		circle.setOrigin(px_radius, px_radius);
		circle.setFillColor(sf::Color::Transparent);
		circle.setOutlineThickness(1.0f);
		circle.setOutlineColor(_b2_to_sf(color));
		_render_target.draw(circle);
	}

	void DebugDrawSFML::DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color)
	{
		// TODO: What is axis?

		float px_center_x = center.x * PIXELS_PER_METER;
		float px_center_y = center.y * PIXELS_PER_METER;
		float px_radius = radius * PIXELS_PER_METER;

		sf::CircleShape circle(px_radius);
		circle.setPosition(px_center_x, px_center_y);
		circle.setOrigin(px_radius, px_radius);
		circle.setFillColor(_b2_to_sf(color));
		_render_target.draw(circle);
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
}

