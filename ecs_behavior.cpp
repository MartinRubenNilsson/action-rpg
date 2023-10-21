#include "ecs_behavior.h"
#include "behavior.h"
#include "physics_helpers.h"
#include "ecs_common.h"
#include "ecs_player.h"

using namespace behavior;

namespace ecs
{
	extern entt::registry _registry;

	struct DestroySelfNode : Node
	{
		entt::entity entity;

		Status update(float dt) override
		{
			ecs::mark_for_destruction(entity);
			return SUCCESS;
		}
	};

	struct StopMovingNode : Node
	{
		entt::handle handle;

		Status update(float dt) override
		{
			if (!handle.valid()) return FAILURE;
			if (!handle.all_of<b2Body*>()) return FAILURE;
			set_linear_velocity(handle.get<b2Body*>(), sf::Vector2f());
			return SUCCESS;
		}
	};

	struct IsPlayerInRangeNode : DecoratorNode
	{
		entt::handle handle;

		Status update(float dt) override
		{
			if (!player_exists()) return FAILURE;
			if (!handle.valid()) return FAILURE;
			if (!handle.all_of<b2Body*>()) return FAILURE;
			float radius;
			if (!get_float(handle.entity(), "radius", radius)) return FAILURE;
			sf::Vector2f center = get_world_center(handle.get<b2Body*>());
			sf::Vector2f player_center = get_player_center();
			if (length(player_center - center) > radius) return FAILURE;
			return child->update(dt);
		}
	};

	struct ApproachPlayerNode : Node
	{
		entt::handle handle;

		Status update(float dt) override
		{
			if (!player_exists()) return FAILURE;
			if (!handle.valid()) return FAILURE;
			if (!handle.all_of<b2Body*>()) return FAILURE;
			float speed;
			if (!get_float(handle.entity(), "speed", speed)) return FAILURE;
			auto body = handle.get<b2Body*>();
			sf::Vector2f position = get_world_center(body);
			sf::Vector2f player_position = get_player_center();
			sf::Vector2f direction = normalize(player_position - position);
			sf::Vector2f velocity = direction * speed;
			set_linear_velocity(body, velocity);
			return SUCCESS;
		}
	};

	Node::Ptr _create_is_player_in_range_node(Node::Ptr child, entt::entity entity)
	{
		auto node = std::make_shared<IsPlayerInRangeNode>();
		node->child = child;
		node->handle = entt::handle(_registry, entity);
		return node;
	}

	Node::Ptr _create_approach_player_node(entt::entity entity)
	{
		auto node = std::make_shared<ApproachPlayerNode>();
		node->handle = entt::handle(_registry, entity);
		return node;
	}

	Node::Ptr _create_stop_moving_node(entt::entity entity)
	{
		auto node = std::make_shared<StopMovingNode>();
		node->handle = entt::handle(_registry, entity);
		return node;
	}

	Node::Ptr _create_enemy_behavior(entt::entity entity)
	{
		return
			create_selector_node({
				_create_is_player_in_range_node(
					_create_approach_player_node(entity), entity),
				_create_stop_moving_node(entity)});
	}

	bool emplace_behavior(entt::entity entity, const std::string& behavior_name)
	{
		if (behavior_name == "enemy")
		{
			_registry.emplace_or_replace<Node::Ptr>(entity, _create_enemy_behavior(entity));
			return true;
		}

		return false;
	}

	void update_behaviors(float dt)
	{
		for (auto [entity, node] : _registry.view<Node::Ptr>().each())
			node->update(dt);
	}
}
