#include "physics_debug.h"

namespace physics
{
	// TODO: frustum culling

	sf::Color _b2_to_sf(const b2Color& color)
	{
		return sf::Color(
			(sf::Uint8)(color.r * 255),
			(sf::Uint8)(color.g * 255),
			(sf::Uint8)(color.b * 255),
			(sf::Uint8)(color.a * 255));
	};

	DebugDrawSFML::DebugDrawSFML(sf::RenderTarget& render_target)
		: _render_target(render_target)
	{}

	void DebugDrawSFML::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
	{
		// TODO
	}

	void DebugDrawSFML::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
	{
		sf::Color sf_color = _b2_to_sf(color);
		sf::VertexArray polygon(sf::LineStrip, vertexCount + 1);
		for (int32 i = 0; i < vertexCount + 1; ++i) {
			const b2Vec2& vertex = vertices[i % vertexCount];
			polygon[i].position = sf::Vector2f(vertex.x, vertex.y);
			polygon[i].color = sf_color;
		}
		_render_target.draw(polygon);
	}

	void DebugDrawSFML::DrawCircle(const b2Vec2& center, float radius, const b2Color& color)
	{
		// TODO
	}

	void DebugDrawSFML::DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color)
	{
		// TODO: What is axis?
		sf::CircleShape circle(radius);
		circle.setPosition(center.x, center.y);
		circle.setOrigin(radius, radius);
		circle.setFillColor(sf::Color::Transparent);
		circle.setOutlineThickness(0.3f);
		circle.setOutlineColor(_b2_to_sf(color));
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
