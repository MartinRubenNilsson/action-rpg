#pragma once

namespace ecs
{
	struct RayHit
	{
		b2Fixture* fixture = nullptr;
		b2Body* body = nullptr;
		entt::entity entity = entt::null;
		sf::Vector2f point;
		sf::Vector2f normal;
		float fraction = 0.f; // 0 = start of ray, 1 = end of ray
	};

	struct BoxHit
	{
		b2Fixture* fixture = nullptr;
		b2Body* body = nullptr;
		entt::entity entity = entt::null;
	};

	void initialize_physics();
	void shutdown_physics();
	void update_physics(float dt);
	void render_physics(sf::RenderTarget& target);

	b2Body* emplace_body(entt::entity entity, const b2BodyDef& body_def);
	void remove_body(entt::entity entity);

	std::vector<RayHit> raycast(const sf::Vector2f& ray_start, const sf::Vector2f& ray_end, uint16 mask_bits = 0xFFFF);
	std::vector<BoxHit> boxcast(const sf::Vector2f& box_min, const sf::Vector2f& box_max, uint16 mask_bits = 0xFFFF);
}

