#pragma once

namespace ecs
{
	enum class CollisionCategory : uint16_t
	{
		None         = 0,
		Default      = 1 << 0,
		Player       = 1 << 1,
		Enemy        = 1 << 2,
		PlayerAttack = 1 << 3,
		EnemyAttack  = 1 << 4,

#pragma warning(push)
#pragma warning(disable: 4369) // warning C4369: enumerator value cannot be represented as 'uint16_t'
#pragma warning(disable: 4309) // warning C4309: truncation of constant value

		PlayerMask = ~PlayerAttack,

#pragma warning(pop)

		// EnTT detects this magic value and enables bitmask operators.
		_entt_enum_as_bitmask
	};

	void initialize_physics();
	void shutdown_physics();
	void update_physics(float dt);
	void render_physics(sf::RenderTarget& target);
	void destroy_slime(entt::entity slime_entity);

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

