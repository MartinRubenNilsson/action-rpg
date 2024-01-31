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

	DebugDrawSFML::DebugDrawSFML(sf::RenderTarget& target)
		: _target(target)
	{
		// Assuming no rotation.
		const sf::View& view = target.getView();
		_view_bounds = sf::FloatRect(view.getCenter() - view.getSize() / 2.0f, view.getSize());
	}

	void DebugDrawSFML::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
	{
		// TODO
	}

	void DebugDrawSFML::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
	{
		bool in_view = false;
		for (int32 i = 0; i < vertexCount; ++i) {
			if (_view_bounds.contains(vertices[i].x, vertices[i].y)) {
				in_view = true;
				break;
			}
		}
		if (!in_view) return;
		sf::Color sf_color = _b2_to_sf(color);
		sf::VertexArray polygon(sf::LineStrip, vertexCount + 1);
		for (int32 i = 0; i < vertexCount + 1; ++i) {
			const b2Vec2& vertex = vertices[i % vertexCount];
			polygon[i].position = sf::Vector2f(vertex.x, vertex.y);
			polygon[i].color = sf_color;
		}
		_target.draw(polygon);
	}

	void DebugDrawSFML::DrawCircle(const b2Vec2& center, float radius, const b2Color& color)
	{
		// TODO
	}

	void DebugDrawSFML::DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color)
	{
		sf::FloatRect circle_bounds(
			center.x - radius,
			center.y - radius,
			radius * 2.0f,
			radius * 2.0f);
		if (!_view_bounds.intersects(circle_bounds)) return;
		// TODO: What is axis?
		sf::CircleShape circle(radius);
		circle.setPosition(center.x, center.y);
		circle.setOrigin(radius, radius);
		circle.setFillColor(sf::Color::Transparent);
		circle.setOutlineThickness(0.3f);
		circle.setOutlineColor(_b2_to_sf(color));
		_target.draw(circle);
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
