#pragma once
#ifdef _DEBUG

namespace shapes {
	const unsigned int MAX_POLYGON_VERTICES = 8;

	void update_lifetimes(float dt);
	void draw_all(std::string_view debug_group_name, const Vector2f& camera_min, const Vector2f& camera_max);

	void add_point(const Vector2f& point, const Color& color = colors::WHITE, float lifetime = 0.f);
	void add_line(const Vector2f& p1, const Vector2f& p2, const Color& color = colors::WHITE, float lifetime = 0.f);
	void add_box(const Vector2f& min, const Vector2f& max, const Color& color = colors::WHITE, float lifetime = 0.f);
	void add_polygon(const Vector2f* points, unsigned int count, const Color& color = colors::WHITE, float lifetime = 0.f);
	void add_circle(const Vector2f& center, float radius, const Color& color = colors::WHITE, float lifetime = 0.f);
}

#endif // _DEBUG