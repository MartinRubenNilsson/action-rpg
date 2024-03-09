#include "stdafx.h"
#include "physics_debug.h"
#include "debug_draw.h"

namespace physics
{
	sf::Vector2f _to_sf(const b2Vec2& vec) {
		return sf::Vector2f(vec.x, vec.y);
	}

	sf::Color _to_sf(const b2Color& color)
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

	void DebugDrawSFML::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) {
		debug::draw_polygon((const sf::Vector2f*)vertices, vertexCount, _to_sf(color));
	}

	void DebugDrawSFML::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) {
		debug::draw_polygon((const sf::Vector2f*)vertices, vertexCount, _to_sf(color));
	}

	void DebugDrawSFML::DrawCircle(const b2Vec2& center, float radius, const b2Color& color) {
		debug::draw_circle(_to_sf(center), radius, _to_sf(color));
	}

	void DebugDrawSFML::DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color) {
		debug::draw_circle(_to_sf(center), radius, _to_sf(color));
	}

	void DebugDrawSFML::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) {
		debug::draw_line(_to_sf(p1), _to_sf(p2), _to_sf(color));
	}

	void DebugDrawSFML::DrawTransform(const b2Transform& xf) {
		std::cerr << "DrawTransform not implemented" << std::endl;
	}

	void DebugDrawSFML::DrawPoint(const b2Vec2& p, float size, const b2Color& color) {
		std::cerr << "DrawPoint not implemented" << std::endl;
	}
}
