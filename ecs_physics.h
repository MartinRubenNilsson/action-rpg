#pragma once

namespace ecs
{
	void initialize_physics();
	void shutdown_physics();
	void update_physics(float dt);
	void render_physics(sf::RenderTarget& target);

	b2Body* emplace_body(entt::entity entity, const b2BodyDef& body_def);
	void remove_body(entt::entity entity);

	struct RayHit
	{
		b2Fixture* fixture = nullptr;
		b2Body* body = nullptr;
		entt::entity entity = entt::null;
		sf::Vector2f point;
		sf::Vector2f normal;
		float fraction = 0.f; // 0 = start, 1 = end
	};

	std::vector<RayHit> raycast(const sf::Vector2f& start, const sf::Vector2f& end);
	std::vector<entt::entity> query_aabb(const sf::Vector2f& min, const sf::Vector2f& max);
}

