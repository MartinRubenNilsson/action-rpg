#pragma once

namespace ecs
{
	struct RaycastHit
	{
		b2Fixture* fixture = nullptr;
		b2Body* body = nullptr;
		entt::entity entity = entt::null;
		sf::Vector2f point;
		sf::Vector2f normal;
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

	bool has_body(entt::entity entity);
	b2Body* emplace_body(entt::entity entity, const b2BodyDef& body_def);
	b2Body* deep_copy_and_emplace_body(entt::entity entity, const b2Body* body);
	b2Body* get_body(entt::entity entity); // WARNING: undefined behavior if entity has no body
	// To be consistent with the other try_get functions, try_get_body should technically
	// return a pointer to a pointer, but that would be a bit awkward to use.
	b2Body* try_get_body(entt::entity entity);
	bool remove_body(entt::entity entity);

	bool raycast(const sf::Vector2f& ray_start, const sf::Vector2f& ray_end, uint16 mask_bits = 0xFFFF, RaycastHit* hit = nullptr);
	std::vector<RaycastHit> raycast_all(const sf::Vector2f& ray_start, const sf::Vector2f& ray_end, uint16 mask_bits = 0xFFFF);
	std::vector<OverlapHit> overlap_box(const sf::Vector2f& box_min, const sf::Vector2f& box_max, uint16 mask_bits = 0xFFFF);
	std::vector<OverlapHit> overlap_circle(const sf::Vector2f& center, float radius, uint16 mask_bits = 0xFFFF);
}

