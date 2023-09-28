#include "behavior_internal.h"
#include "ecs_player.h"
#include "physics_helpers.h"

namespace behavior
{
	struct IsPlayerInRange : BT::ConditionNode, EntityNode
	{
		IsPlayerInRange(const std::string& name, const BT::NodeConfig& config)
			: BT::ConditionNode(name, config)
		{}

		static BT::PortsList providedPorts() {
			return { BT::InputPort<float>("radius", 0.f, "The radius of the range") };
		}

		BT::NodeStatus tick() override
		{
			if (!ecs::player_exists()) return BT::NodeStatus::FAILURE;
			if (!handle.all_of<b2Body*>()) return BT::NodeStatus::FAILURE;
			auto body = handle.get<b2Body*>();

			auto radius = getInput<float>("radius");
			if (!radius) return BT::NodeStatus::FAILURE;

			sf::Vector2f center = get_world_center(body);
			sf::Vector2f player_center = ecs::get_player_center();
			float distance = length(player_center - center);

			return (distance <= radius.value()) ?
				BT::NodeStatus::SUCCESS :
				BT::NodeStatus::FAILURE;
		}
	};

	struct ApproachPlayer : BT::SyncActionNode, EntityNode
	{
		ApproachPlayer(const std::string& name, const BT::NodeConfig& config)
			: BT::SyncActionNode(name, config)
		{}

		static BT::PortsList providedPorts() {
			return { BT::InputPort<float>("speed", 0.f, "The movement speed to use") };
		}

		BT::NodeStatus tick() override
		{
			if (!ecs::player_exists()) return BT::NodeStatus::FAILURE;
			if (!handle.all_of<b2Body*>()) return BT::NodeStatus::FAILURE;
			auto body = handle.get<b2Body*>();

			auto speed = getInput<float>("speed");
			if (!speed) return BT::NodeStatus::FAILURE;

			sf::Vector2f position = get_world_center(body);
			sf::Vector2f player_position = ecs::get_player_center();
			sf::Vector2f direction = normalize(player_position - position);
			sf::Vector2f velocity = direction * speed.value();
			set_linear_velocity(body, velocity);

			return BT::NodeStatus::SUCCESS;
		}
	};

	void _register_nodes_player(BT::BehaviorTreeFactory& factory)
	{
		factory.registerNodeType<IsPlayerInRange>("IsPlayerInRange");
		factory.registerNodeType<ApproachPlayer>("ApproachPlayer");
	}
}