#pragma once
#pragma warning(push)
#pragma warning(disable: 4369) // warning C4369: enumerator value cannot be represented as 'uint16_t'
#pragma warning(disable: 4309) // warning C4309: truncation of constant value

namespace ecs
{
	enum CollisionCategory : uint16_t
	{
		CC_None         = 0,
		CC_Default      = 1 << 0,
		CC_Player       = 1 << 1,
		CC_Enemy        = 1 << 2,
		CC_PlayerAttack = 1 << 3,
		CC_EnemyAttack  = 1 << 4,
	};
}

// Enable bitmask operators for CollisionCategory
template<> struct entt::enum_as_bitmask<ecs::CollisionCategory> : std::true_type {};

namespace ecs
{
	enum CollisionMask : uint16_t
	{
		CM_Default = ~CC_None,
		CM_Player = ~CC_PlayerAttack,
	};
}

// Enable bitmask operators for CollisionMask
template<> struct entt::enum_as_bitmask<ecs::CollisionMask> : std::true_type {};

namespace ecs
{
	void initialize_physics();
	void shutdown_physics();
	void update_physics(float dt);
	void render_physics(sf::RenderTarget& target);

	//TODO: put in own header
	void destroy_slime(entt::entity slime_entity);

	b2Body* emplace_body(entt::entity entity, const b2BodyDef& body_def);
	void remove_body(entt::entity entity);

	b2Filter get_filter_for_class(const std::string& class_);

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

#pragma warning(pop)

