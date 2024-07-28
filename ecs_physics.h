#pragma once

namespace ecs
{
	struct RaycastHit
	{
		b2Fixture* fixture = nullptr;
		b2Body* body = nullptr;
		entt::entity entity = entt::null;
		Vector2f point;
		Vector2f normal;
		float fraction = 0.f; // 0 = start of ray, 1 = end of ray
	};

	struct OverlapHit
	{
		b2Fixture* fixture = nullptr;
		b2Body* body = nullptr;
		entt::entity entity = entt::null;
	};

	void initialize_physics();
	void shutdown_physics();
	void update_physics(float dt);
	void debug_draw_physics();

	bool raycast(const Vector2f& ray_start, const Vector2f& ray_end, uint16 mask_bits = 0xFFFF, RaycastHit* hit = nullptr);
	std::vector<RaycastHit> raycast_all(const Vector2f& ray_start, const Vector2f& ray_end, uint16 mask_bits = 0xFFFF);
	std::vector<OverlapHit> overlap_box(const Vector2f& box_min, const Vector2f& box_max, uint16 mask_bits = 0xFFFF);
	std::vector<OverlapHit> overlap_circle(const Vector2f& center, float radius, uint16 mask_bits = 0xFFFF);

	b2Body* emplace_body(entt::entity entity, const b2BodyDef& body_def);
	b2Body* deep_copy_and_emplace_body(entt::entity entity, const b2Body* body);
	// To be consistent with the other get_*() functions, get_body() should technically
	// return a pointer to a pointer, but that would be a bit awkward to use.
	b2Body* get_body(entt::entity entity);
	bool remove_body(entt::entity entity);

	b2FixtureDef get_fixture_def(const b2Fixture* fixture);
	b2BodyDef get_body_def(const b2Body* body);
	void set_world_center(b2Body* body, const Vector2f& center); // Center of mass.
	// Sets the category bits of all fixtures attached to the body.
	void set_category_bits(b2Body* body, uint32_t category_bits);
	// Returns the bitwise OR of all category bits of all fixtures attached to the body.
	uint32_t get_category_bits(const b2Body* body);
}

